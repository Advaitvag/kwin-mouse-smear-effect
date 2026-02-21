#!/bin/bash

# KWin Mouse Smear Effect Uninstaller

EFFECT_ID="mouse-smear"

echo "Disabling effect in kwinrc..."
kwriteconfig6 --file kwinrc --group Plugins --key "${EFFECT_ID}Enabled" false

echo "Removing $EFFECT_ID from user plugins..."
kpackagetool6 --type KWin/Effect --remove "$EFFECT_ID"

if [ $? -eq 0 ]; then
    echo "Effect uninstalled successfully."
    
    echo "Reloading KWin configuration..."
    qdbus6 org.kde.KWin /KWin org.kde.KWin.reconfigure
    
    echo "Done!"
else
    echo "Failed to uninstall the effect."
    exit 1
fi
