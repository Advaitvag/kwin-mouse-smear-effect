#!/bin/bash

# KWin Mouse Smear Effect Installer (C++ version)
# Requires cmake, g++, and kwin development files.

EFFECT_ID="mouse-smear"
PLUGIN_NAME="kwin_mouse_smear"

echo "Uninstalling QML version if any..."
kpackagetool6 --type KWin/Effect --remove "$EFFECT_ID" 2>/dev/null

echo "Building C++ version..."
mkdir -p build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr
make -j$(nproc)

if [ $? -ne 0 ]; then
    echo "Build failed."
    exit 1
fi

echo "Installing C++ plugin..."
# We use sudo because it installs to /usr/lib
sudo make install

echo "Enabling effect in kwinrc..."
kwriteconfig6 --file kwinrc --group Plugins --key "${EFFECT_ID}Enabled" true

echo "Reloading KWin configuration..."
qdbus6 org.kde.KWin /KWin org.kde.KWin.reconfigure

echo "Done! The C++ Mouse Smear effect should now be active."
echo "You can customize it in System Settings -> Desktop Effects."
