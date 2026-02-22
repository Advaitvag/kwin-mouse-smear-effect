#pragma once

#include "effect/effect.h"
#include "opengl/glutils.h"
#include <QList>
#include <QPointF>
#include <QColor>
#include <QDateTime>
#include <memory>

namespace KWin
{

class SmearConfig;

struct TrailPoint
{
    QPointF pos;
    qint64 timestamp;
};

class MouseSmearEffect : public Effect
{
    Q_OBJECT
public:
    MouseSmearEffect();
    ~MouseSmearEffect() override;

    static bool supported();
    static bool enabledByDefault();

    void reconfigure(ReconfigureFlags) override;
    void paintScreen(const RenderTarget &renderTarget, const RenderViewport &viewport, int mask, const Region &deviceRegion, LogicalOutput *screen) override;
    bool isActive() const override;
    int requestedEffectChainPosition() const override;

private Q_SLOTS:
    void slotMouseChanged(const QPointF &pos, const QPointF &old,
                          Qt::MouseButtons buttons, Qt::MouseButtons oldbuttons,
                          Qt::KeyboardModifiers modifiers, Qt::KeyboardModifiers oldmodifiers);

private:
    QList<TrailPoint> m_trail;
};

} // namespace KWin
