#include "smear_config.h"
#include "smearconfig.h"
#include <KPluginFactory>
#include <QComboBox>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>

namespace KWin
{

K_PLUGIN_CLASS(MouseSmearEffectConfig)

MouseSmearEffectConfig::MouseSmearEffectConfig(QObject *parent, const KPluginMetaData &data)
    : KCModule(parent, data)
{
    m_ui.setupUi(widget());

    addConfig(SmearConfig::self(), widget());

    auto updateColorEnabled = [this] {
        m_ui.kcfg_TrailColor->setEnabled(m_ui.kcfg_RainbowMode->currentIndex() == 0);
    };
    connect(m_ui.kcfg_RainbowMode, &QComboBox::currentIndexChanged, updateColorEnabled);
    updateColorEnabled();
}

void MouseSmearEffectConfig::save()
{
    KCModule::save();

    QDBusMessage message = QDBusMessage::createMethodCall(QStringLiteral("org.kde.KWin"),
                                                          QStringLiteral("/Effects"),
                                                          QStringLiteral("org.kde.kwin.Effects"),
                                                          QStringLiteral("reconfigureEffect"));
    message << QStringLiteral("mouse-smear");
    QDBusConnection::sessionBus().send(message);
}

} // namespace KWin

#include "smear_config.moc"
