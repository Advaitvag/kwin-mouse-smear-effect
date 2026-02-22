#!/bin/bash

# KWin Mouse Trail Effect Uninstaller

EFFECT_ID="mouse-trail"

echo "Disabling effect in kwinrc..."
kwriteconfig6 --file kwinrc --group Plugins --key "${EFFECT_ID}Enabled" false
# Also disable old IDs just in case
kwriteconfig6 --file kwinrc --group Plugins --key "mouse_trailEnabled" false
kwriteconfig6 --file kwinrc --group Plugins --key "mouse_smearEnabled" false
kwriteconfig6 --file kwinrc --group Plugins --key "kwin_mouse_smearEnabled" false

echo "Removing plugin files..."

if [ -f build/install_manifest.txt ]; then
    echo "Using install_manifest.txt for precise removal..."
    sudo xargs rm -fv < build/install_manifest.txt
else
    echo "Warning: build/install_manifest.txt not found. Falling back to manual removal."
    # Fallback to manual removal with more common paths
    # We try to remove all variants (hyphen and underscore)
    
    # Plugin binaries
    PATHS=(
        "/usr/lib/qt6/plugins/kwin/effects/plugins/mouse-trail.so"
        "/usr/lib/qt6/plugins/kwin/effects/plugins/mouse_trail.so"
        "/usr/lib/qt6/plugins/kwin/effects/plugins/mouse-smear.so"
        "/usr/lib/qt6/plugins/kwin/effects/plugins/mouse_smear.so"
        "/usr/lib/qt6/plugins/kwin/effects/plugins/kwin_mouse_smear.so"
        "/usr/lib/x86_64-linux-gnu/qt6/plugins/kwin/effects/plugins/mouse-trail.so"
        "/usr/lib/x86_64-linux-gnu/qt6/plugins/kwin/effects/plugins/mouse_trail.so"
        "/usr/lib/x86_64-linux-gnu/qt6/plugins/kwin/effects/plugins/mouse-smear.so"
        "/usr/lib/x86_64-linux-gnu/qt6/plugins/kwin/effects/plugins/mouse_smear.so"
        "/usr/lib/x86_64-linux-gnu/qt6/plugins/kwin/effects/plugins/kwin_mouse_smear.so"
        "/usr/lib64/qt6/plugins/kwin/effects/plugins/mouse-trail.so"
        "/usr/lib64/qt6/plugins/kwin/effects/plugins/mouse_trail.so"
        "/usr/lib64/qt6/plugins/kwin/effects/plugins/mouse-smear.so"
        "/usr/lib64/qt6/plugins/kwin/effects/plugins/mouse_smear.so"
        "/usr/lib64/qt6/plugins/kwin/effects/plugins/kwin_mouse_smear.so"
        
        "/usr/lib/qt6/plugins/kwin/effects/configs/mouse-trail-config.so"
        "/usr/lib/qt6/plugins/kwin/effects/configs/mouse_trail_config.so"
        "/usr/lib/qt6/plugins/kwin/effects/configs/mouse-smear-config.so"
        "/usr/lib/qt6/plugins/kwin/effects/configs/mouse_smear_config.so"
        "/usr/lib/qt6/plugins/kwin/effects/configs/kwin_mouse_smear_config.so"
        "/usr/lib/x86_64-linux-gnu/qt6/plugins/kwin/effects/configs/mouse-trail-config.so"
        "/usr/lib/x86_64-linux-gnu/qt6/plugins/kwin/effects/configs/mouse_trail_config.so"
        "/usr/lib/x86_64-linux-gnu/qt6/plugins/kwin/effects/configs/mouse-smear-config.so"
        "/usr/lib/x86_64-linux-gnu/qt6/plugins/kwin/effects/configs/mouse_smear_config.so"
        "/usr/lib/x86_64-linux-gnu/qt6/plugins/kwin/effects/configs/kwin_mouse_smear_config.so"
        "/usr/lib64/qt6/plugins/kwin/effects/configs/mouse-trail-config.so"
        "/usr/lib64/qt6/plugins/kwin/effects/configs/mouse_trail_config.so"
        "/usr/lib64/qt6/plugins/kwin/effects/configs/mouse-smear-config.so"
        "/usr/lib64/qt6/plugins/kwin/effects/configs/mouse_smear_config.so"
        "/usr/lib64/qt6/plugins/kwin/effects/configs/kwin_mouse_smear_config.so"
    )

    for path in "${PATHS[@]}"; do
        if [ -f "$path" ]; then
            sudo rm -fv "$path"
        fi
    done
fi

# Data directories (always try to remove these as manifest might not include the dir itself)
sudo rm -rf /usr/share/kwin/effects/mouse-trail/
sudo rm -rf /usr/share/kwin/effects/mouse-smear/
sudo rm -rf /usr/share/kwin/effects/mouse_smear/
sudo rm -rf /usr/share/kwin/effects/kwin_mouse_smear/

echo "Unloading effect from KWin..."
qdbus6 org.kde.KWin /Effects org.kde.kwin.Effects.unloadEffect "${EFFECT_ID}"
qdbus6 org.kde.KWin /Effects org.kde.kwin.Effects.unloadEffect "mouse_smear"
qdbus6 org.kde.KWin /Effects org.kde.kwin.Effects.unloadEffect "kwin_mouse_smear"

echo "Reloading KWin configuration..."
qdbus6 org.kde.KWin /KWin org.kde.KWin.reconfigure

echo "Done! The Mouse Trail effect has been uninstalled."
