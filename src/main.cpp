#include "smear.h"
#include <KPluginFactory>

namespace KWin
{

KWIN_EFFECT_FACTORY_SUPPORTED_ENABLED(MouseSmearEffect,
                                      "../metadata.json",
                                      return MouseSmearEffect::supported();
                                      ,
                                      return MouseSmearEffect::enabledByDefault();)

} // namespace KWin

#include "main.moc"
