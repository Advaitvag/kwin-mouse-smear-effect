#include "trail_config.h"
#include "trailconfig.h"
#include <KPluginFactory>
#include <QComboBox>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>

namespace KWin
{

K_PLUGIN_CLASS_WITH_JSON(MouseTrailEffectConfig, "../metadata.json")

MouseTrailEffectConfig::MouseTrailEffectConfig(QObject *parent, const KPluginMetaData &data)
    : KCModule(parent, data)
    , m_data(data)
{
    m_ui.setupUi(widget());

    TrailConfig::instance(KSharedConfig::openConfig(QStringLiteral("kwinrc")));
    addConfig(TrailConfig::self(), widget());

    auto updateColorEnabled = [this] {
        m_ui.kcfg_TrailColor->setEnabled(m_ui.kcfg_RainbowMode->currentIndex() == 0);
    };
    connect(m_ui.kcfg_RainbowMode, &QComboBox::currentIndexChanged, updateColorEnabled);
    updateColorEnabled();
}

void MouseTrailEffectConfig::save()
{
    qDebug() << "MouseTrailEffectConfig: Saving configuration";
    KCModule::save();

    QDBusInterface interface(QStringLiteral("org.kde.KWin"),
                             QStringLiteral("/Effects"),
                             QStringLiteral("org.kde.kwin.Effects"),
                             QDBusConnection::sessionBus());
    interface.call(QStringLiteral("reconfigureEffect"), QStringLiteral("mouse-trail"));
}

} // namespace KWin

#include "trail_config.moc"
