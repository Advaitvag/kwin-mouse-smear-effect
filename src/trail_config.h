#pragma once

#include "ui_trail_config.h"
#include <KCModule>

namespace KWin
{

class MouseTrailEffectConfig : public KCModule
{
    Q_OBJECT
public:
    explicit MouseTrailEffectConfig(QObject *parent, const KPluginMetaData &data);

    void save() override;

private:
    Ui::MouseTrailConfigForm m_ui;
    KPluginMetaData m_data;
};

} // namespace KWin
