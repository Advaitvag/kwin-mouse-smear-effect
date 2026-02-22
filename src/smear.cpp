#include "smear.h"
#include "smearconfig.h"
#include "core/rendertarget.h"
#include "core/renderviewport.h"
#include "effect/effecthandler.h"
#include "opengl/glutils.h"
#include <QDateTime>
#include <cmath>
#include <QDebug>

namespace KWin
{

MouseSmearEffect::MouseSmearEffect()
{
    qDebug() << "MouseSmearEffect: Initializing C++ version";
    SmearConfig::instance(effects->config());
    connect(effects, &EffectsHandler::mouseChanged, this, &MouseSmearEffect::slotMouseChanged);
    reconfigure(ReconfigureAll);
}

MouseSmearEffect::~MouseSmearEffect()
{
}

bool MouseSmearEffect::supported()
{
    return effects->openglContext() != nullptr;
}

bool MouseSmearEffect::enabledByDefault()
{
    return true;
}

void MouseSmearEffect::reconfigure(ReconfigureFlags)
{
    effects->config()->reparseConfiguration();
    SmearConfig::self()->read();
    m_trail.clear();
    qDebug() << "MouseSmearEffect: Reconfigured. ID: mouse-smear";
    effects->addRepaintFull();
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
    int lifespan = SmearConfig::self()->trailLifeSpan();
    
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

                QColor color = SmearConfig::self()->trailColor();
                if (SmearConfig::self()->rainbowMode() == 1) {
                    color.setHsv((p1.timestamp / 10) % 360, 200, 255);
                }
                color.setAlphaF(ageFactor);

                binder.shader()->setUniform(GLShader::ColorUniform::Color, color);
                glLineWidth(std::max(1.0f, (float)SmearConfig::self()->trailSize() * ageFactor));

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

bool MouseSmearEffect::isActive() const
{
    return !m_trail.isEmpty() && !effects->isScreenLocked();
}

int MouseSmearEffect::requestedEffectChainPosition() const
{
    return 10;
}

} // namespace KWin
