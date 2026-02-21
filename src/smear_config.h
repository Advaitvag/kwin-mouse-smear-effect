#pragma once

#include "ui_smear_config.h"
#include <KCModule>

namespace KWin
{

class MouseSmearEffectConfig : public KCModule
{
    Q_OBJECT
public:
    explicit MouseSmearEffectConfig(QObject *parent, const KPluginMetaData &data);

    void save() override;

private:
    Ui::MouseSmearConfigForm m_ui;
    KPluginMetaData m_data;
};

} // namespace KWin
