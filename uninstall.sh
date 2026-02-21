#!/bin/bash

# KWin Mouse Smear Effect Uninstaller

EFFECT_ID="kwin_mouse_smear"

echo "Disabling effect in kwinrc..."
kwriteconfig6 --file kwinrc --group Plugins --key "${EFFECT_ID}Enabled" false

echo "Removing plugin files..."
sudo rm -f /usr/lib/kwin/effects/plugins/kwin_mouse_smear.so
sudo rm -f /usr/lib/kwin/effects/configs/kwin_mouse_smear_config.so
sudo rm -rf /usr/share/kwin/effects/mouse-smear/

echo "Unloading effect from KWin..."
qdbus6 org.kde.KWin /Effects org.kde.kwin.Effects.unloadEffect "${EFFECT_ID}"

echo "Reloading KWin configuration..."
qdbus6 org.kde.KWin /KWin org.kde.KWin.reconfigure

echo "Done! The Mouse Smear effect has been uninstalled."
