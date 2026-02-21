#include "smear.h"
#include "smearconfig.h"
#include "core/rendertarget.h"
#include "core/renderviewport.h"
#include "effect/effecthandler.h"
#include "opengl/glutils.h"
#include <QDateTime>
#include <cmath>
#include <QSvgRenderer>
#include <QPainter>
#include <QStandardPaths>
#include <QDebug>

namespace KWin
{

MouseSmearEffect::MouseSmearEffect()
    : m_config(std::make_unique<SmearConfig>())
{
    qDebug() << "MouseSmearEffect: Initializing C++ version";
    connect(effects, &EffectsHandler::mouseChanged, this, &MouseSmearEffect::slotMouseChanged);
    reconfigure(ReconfigureAll);
}

MouseSmearEffect::~MouseSmearEffect()
{
}

void MouseSmearEffect::reconfigure(ReconfigureFlags)
{
    m_config->config()->reparseConfiguration();
    m_config->read();
    m_trail.clear();
    m_dotTexture.reset();
    m_starTexture.reset();
    qDebug() << "MouseSmearEffect: Reconfigured. ID: kwin_mouse_smear";
    effects->addRepaintFull();
}

void MouseSmearEffect::ensureTextures()
{
    if (m_dotTexture && m_starTexture) {
        return;
    }

    QString assetsPath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, 
                                                QStringLiteral("kwin/effects/mouse-smear/contents/assets"), 
                                                QStandardPaths::LocateDirectory);
    if (assetsPath.isEmpty()) {
        assetsPath = QStringLiteral("/usr/share/kwin/effects/mouse-smear/contents/assets");
    }

    qDebug() << "MouseSmearEffect: Loading textures from" << assetsPath;

    auto loadTexture = [](const QString &path, int size) {
        QSvgRenderer renderer(path);
        if (!renderer.isValid()) {
            qWarning() << "MouseSmearEffect: Failed to load SVG from" << path;
            return std::unique_ptr<GLTexture>();
        }
        QImage image(size, size, QImage::Format_ARGB32);
        image.fill(Qt::transparent);
        QPainter painter(&image);
        renderer.render(&painter);
        return std::unique_ptr<GLTexture>(GLTexture::upload(image));
    };

    if (!m_dotTexture) {
        m_dotTexture = loadTexture(assetsPath + QStringLiteral("/dot.svg"), 128);
    }
    if (!m_starTexture) {
        m_starTexture = loadTexture(assetsPath + QStringLiteral("/star.svg"), 128);
    }
}

void MouseSmearEffect::slotMouseChanged(const QPointF &pos, const QPointF &,
                                        Qt::MouseButtons, Qt::MouseButtons,
                                        Qt::KeyboardModifiers, Qt::KeyboardModifiers)
{
    if (effects->isScreenLocked()) {
        return;
    }

    m_trail.append({pos, QDateTime::currentMSecsSinceEpoch()});
    effects->addRepaintFull();
}

void MouseSmearEffect::paintScreen(const RenderTarget &renderTarget, const RenderViewport &viewport, int mask, const Region &deviceRegion, LogicalOutput *screen)
{
    effects->paintScreen(renderTarget, viewport, mask, deviceRegion, screen);

    qint64 now = QDateTime::currentMSecsSinceEpoch();
    int lifespan = m_config->trailLifeSpan();
    
    // Cleanup old points
    while (!m_trail.isEmpty() && now - m_trail.first().timestamp > lifespan) {
        m_trail.removeFirst();
    }

    if (m_trail.isEmpty()) {
        return;
    }

    ensureTextures();

    if (effects->openglContext()) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        const auto scale = viewport.scale();
        
        ShaderBinder binder(ShaderTrait::UniformColor | ShaderTrait::TransformColorspace);
        binder.shader()->setUniform(GLShader::Mat4Uniform::ModelViewProjectionMatrix, viewport.projectionMatrix());
        binder.shader()->setColorspaceUniforms(ColorDescription::sRGB, renderTarget.colorDescription(), RenderingIntent::Perceptual);

        GLVertexBuffer *vbo = GLVertexBuffer::streamingBuffer();
        vbo->reset();
        vbo->setAttribLayout(std::span(GLVertexBuffer::GLVertex2DLayout), sizeof(GLVertex2D));

        // Draw segments
        for (int i = 0; i < m_trail.size() - 1; ++i) {
            const auto &p1 = m_trail[i];
            const auto &p2 = m_trail[i+1];
            
            float ageFactor = 1.0f - (float)(now - p1.timestamp) / lifespan;
            if (ageFactor < 0) ageFactor = 0;

            QColor color = m_config->trailColor();
            if (m_config->rainbowMode() == 1) {
                color.setHsv((p1.timestamp / 10) % 360, 200, 255);
            }
            color.setAlphaF(ageFactor);

            binder.shader()->setUniform(GLShader::ColorUniform::Color, color);
            glLineWidth(std::max(1.0f, (float)m_config->trailSize() * ageFactor));

            if (auto result = vbo->map<GLVertex2D>(2)) {
                auto map = *result;
                map[0] = GLVertex2D{ .position = QVector2D(p1.pos.x() * scale, p1.pos.y() * scale), .texcoord = QVector2D() };
                map[1] = GLVertex2D{ .position = QVector2D(p2.pos.x() * scale, p2.pos.y() * scale), .texcoord = QVector2D() };
                vbo->unmap();
                vbo->bindArrays();
                vbo->draw(GL_LINES, 0, 2);
                vbo->unbindArrays();
            }
        }

        // Draw sparkles
        if (m_config->sparkleMode()) {
            ShaderBinder texBinder(ShaderTrait::MapTexture | ShaderTrait::TransformColorspace);
            texBinder.shader()->setUniform(GLShader::Mat4Uniform::ModelViewProjectionMatrix, viewport.projectionMatrix());
            texBinder.shader()->setColorspaceUniforms(ColorDescription::sRGB, renderTarget.colorDescription(), RenderingIntent::Perceptual);
            
            if (m_starTexture) {
                m_starTexture->bind();
                for (const auto &p : m_trail) {
                    if ((p.timestamp % 100) < 20) {
                        float ageFactor = 1.0f - (float)(now - p.timestamp) / lifespan;
                        float size = 16.0f * ageFactor;
                        
                        float offsetX = sin(p.timestamp / 100.0) * m_config->wander();
                        float offsetY = (now - p.timestamp) * 0.001 * m_config->gravity();
                        
                        QRectF rect(p.pos.x() * scale - size/2 + offsetX, p.pos.y() * scale - size/2 + offsetY, size, size);
                        
                        if (auto result = vbo->map<GLVertex2D>(6)) {
                            auto map = *result;
                            map[0] = GLVertex2D{ .position = QVector2D(rect.left(), rect.top()), .texcoord = QVector2D(0, 1) };
                            map[1] = GLVertex2D{ .position = QVector2D(rect.right(), rect.top()), .texcoord = QVector2D(1, 1) };
                            map[2] = GLVertex2D{ .position = QVector2D(rect.left(), rect.bottom()), .texcoord = QVector2D(0, 0) };
                            map[3] = GLVertex2D{ .position = QVector2D(rect.left(), rect.bottom()), .texcoord = QVector2D(0, 0) };
                            map[4] = GLVertex2D{ .position = QVector2D(rect.right(), rect.top()), .texcoord = QVector2D(1, 1) };
                            map[5] = GLVertex2D{ .position = QVector2D(rect.right(), rect.bottom()), .texcoord = QVector2D(1, 0) };
                            vbo->unmap();
                            vbo->bindArrays();
                            vbo->draw(GL_TRIANGLES, 0, 6);
                            vbo->unbindArrays();
                        }
                    }
                }
            }
        }

        glLineWidth(1.0);
        glDisable(GL_BLEND);
    }
    
    effects->addRepaintFull();
}

bool MouseSmearEffect::isActive() const
{
    return !m_trail.isEmpty() && !effects->isScreenLocked();
}

int MouseSmearEffect::requestedEffectChainPosition() const
{
    return 10;
}

} // namespace KWin
