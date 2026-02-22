#!/bin/bash

# KWin Mouse Trail Effect Installer
# Requires cmake, g++, and kwin development files.

EFFECT_ID="mouse-trail"

echo "Building..."
mkdir -p build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr
make -j$(nproc)

if [ $? -ne 0 ]; then
    echo "Build failed."
    exit 1
fi

echo "Installing plugin..."
# We use sudo because it installs to /usr/lib
sudo make install

echo "Enabling effect in kwinrc..."
kwriteconfig6 --file kwinrc --group Plugins --key "${EFFECT_ID}Enabled" true

echo "Reloading KWin configuration..."
qdbus6 org.kde.KWin /KWin org.kde.KWin.reconfigure

echo "Explicitly loading the effect..."
qdbus6 org.kde.KWin /Effects org.kde.kwin.Effects.loadEffect "${EFFECT_ID}"

echo "Done! The Mouse Trail effect should now be active."
echo "You can customize it in System Settings -> Desktop Effects."
