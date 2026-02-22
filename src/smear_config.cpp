#include "smear_config.h"
#include "smearconfig.h"
#include <KPluginFactory>
#include <QComboBox>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>

namespace KWin
{

K_PLUGIN_CLASS_WITH_JSON(MouseSmearEffectConfig, "../metadata.json")

MouseSmearEffectConfig::MouseSmearEffectConfig(QObject *parent, const KPluginMetaData &data)
    : KCModule(parent, data)
    , m_data(data)
{
    m_ui.setupUi(widget());

    SmearConfig::instance(KSharedConfig::openConfig(QStringLiteral("kwinrc")));
    addConfig(SmearConfig::self(), widget());

    auto updateColorEnabled = [this] {
        m_ui.kcfg_TrailColor->setEnabled(m_ui.kcfg_RainbowMode->currentIndex() == 0);
    };
    connect(m_ui.kcfg_RainbowMode, &QComboBox::currentIndexChanged, updateColorEnabled);
    updateColorEnabled();
}

void MouseSmearEffectConfig::save()
{
    qDebug() << "MouseSmearEffectConfig: Saving configuration";
    KCModule::save();

    QDBusInterface interface(QStringLiteral("org.kde.KWin"),
                             QStringLiteral("/Effects"),
                             QStringLiteral("org.kde.kwin.Effects"),
                             QDBusConnection::sessionBus());
    interface.call(QStringLiteral("reconfigureEffect"), QStringLiteral("mouse-smear"));
}

} // namespace KWin

#include "smear_config.moc"
