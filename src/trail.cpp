#include "trail.h"
#include "trailconfig.h"
#include "core/rendertarget.h"
#include "core/renderviewport.h"
#include "effect/effecthandler.h"
#include "opengl/glutils.h"
#include <QDateTime>
#include <cmath>
#include <QDebug>

namespace KWin
{

MouseTrailEffect::MouseTrailEffect()
{
    qDebug() << "MouseTrailEffect: Initializing C++ version";
    TrailConfig::instance(effects->config());
    connect(effects, &EffectsHandler::mouseChanged, this, &MouseTrailEffect::slotMouseChanged);
    reconfigure(ReconfigureAll);
}

MouseTrailEffect::~MouseTrailEffect()
{
}

bool MouseTrailEffect::supported()
{
    return effects->openglContext() != nullptr;
}

bool MouseTrailEffect::enabledByDefault()
{
    return true;
}

void MouseTrailEffect::reconfigure(ReconfigureFlags)
{
    effects->config()->reparseConfiguration();
    TrailConfig::self()->read();
    m_trail.clear();
    qDebug() << "MouseTrailEffect: Reconfigured. ID: mouse-trail";
    effects->addRepaintFull();
}

void MouseTrailEffect::slotMouseChanged(const QPointF &pos, const QPointF &,
                                        Qt::MouseButtons, Qt::MouseButtons,
                                        Qt::KeyboardModifiers, Qt::KeyboardModifiers)
{
    if (effects->isScreenLocked()) {
        return;
    }

    m_trail.append({pos, QDateTime::currentMSecsSinceEpoch()});
    effects->addRepaintFull();
}

void MouseTrailEffect::paintScreen(const RenderTarget &renderTarget, const RenderViewport &viewport, int mask, const Region &deviceRegion, LogicalOutput *screen)
{
    effects->paintScreen(renderTarget, viewport, mask, deviceRegion, screen);

    qint64 now = QDateTime::currentMSecsSinceEpoch();
    int lifespan = TrailConfig::self()->trailLifeSpan();
    
    // Cleanup old points
    while (!m_trail.isEmpty() && now - m_trail.first().timestamp > lifespan) {
        m_trail.removeFirst();
    }

    if (m_trail.isEmpty()) {
        return;
    }

    if (effects->openglContext()) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        const auto scale = viewport.scale();
        
        GLVertexBuffer *vbo = GLVertexBuffer::streamingBuffer();

        // Draw segments
        if (m_trail.size() >= 2) {
            ShaderBinder binder(ShaderTrait::UniformColor | ShaderTrait::TransformColorspace);
            binder.shader()->setUniform(GLShader::Mat4Uniform::ModelViewProjectionMatrix, viewport.projectionMatrix());
            binder.shader()->setColorspaceUniforms(ColorDescription::sRGB, renderTarget.colorDescription(), RenderingIntent::Perceptual);
            
            vbo->reset();
            vbo->setAttribLayout(std::span(GLVertexBuffer::GLVertex2DLayout), sizeof(GLVertex2D));
            
            for (int i = 0; i < m_trail.size() - 1; ++i) {
                const auto &p1 = m_trail[i];
                const auto &p2 = m_trail[i+1];
                
                float ageFactor = 1.0f - (float)(now - p1.timestamp) / lifespan;
                if (ageFactor < 0) ageFactor = 0;

                QColor color = TrailConfig::self()->trailColor();
                if (TrailConfig::self()->rainbowMode() == 1) {
                    color.setHsv((p1.timestamp / 10) % 360, 200, 255);
                }
                color.setAlphaF(ageFactor);

                binder.shader()->setUniform(GLShader::ColorUniform::Color, color);
                glLineWidth(std::max(1.0f, (float)TrailConfig::self()->trailSize() * ageFactor));

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
        }

        glLineWidth(1.0);
        glDisable(GL_BLEND);
    }
    
    if (!m_trail.isEmpty()) {
        effects->addRepaintFull();
    }
}

bool MouseTrailEffect::isActive() const
{
    return !m_trail.isEmpty() && !effects->isScreenLocked();
}

int MouseTrailEffect::requestedEffectChainPosition() const
{
    return 10;
}

} // namespace KWin
