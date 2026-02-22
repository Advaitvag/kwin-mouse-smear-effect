#include "trail.h"
#include <KPluginFactory>

namespace KWin
{

KWIN_EFFECT_FACTORY_SUPPORTED_ENABLED(MouseTrailEffect,
                                      "../metadata.json",
                                      return MouseTrailEffect::supported();
                                      ,
                                      return MouseTrailEffect::enabledByDefault();)

} // namespace KWin

#include "main.moc"
