# Mouse Trail KWin Effect

An aesthetically pleasing mouse cursor trail effect for KDE Plasma 6.

## Features

- **Customizable Trail**: Adjust color, decay rate (lifespan), and size.
- **Rainbow Mode**: Cycles through a spectrum of colors.
- **High Performance**: Native C++ implementation for smooth rendering.

## Requirements

### Runtime

- KDE Plasma 6.0 or higher.
- `kwriteconfig6` and `qdbus6` (standard KDE utilities).

### Build Requirements

To build from source, you will need:

- **CMake** (>= 3.16)
- **C++ Compiler** (supporting C++20)
- **Extra CMake Modules (ECM)**
- **KWin Development Files**
- **Qt6 Libraries** (Quick, Widgets, DBus)
- **KDE Frameworks 6 (KF6)** (Config, ConfigWidgets, GlobalAccel, I18n, CoreAddons, KCMUtils)

On Fedora, you might need: `dnf install cmake extra-cmake-modules kwin-devel qt6-qtquick-devel kf6-kconfig-devel kf6-kconfigwidgets-devel kf6-kglobalaccel-devel kf6-ki18n-devel kf6-kcoreaddons-devel kf6-kcmutils-devel`
On Ubuntu/KDE Neon: `apt install cmake extra-cmake-modules kwin-dev qt6-base-dev qt6-declarative-dev libkf6config-dev libkf6configwidgets-dev libkf6globalaccel-dev libkf6i18n-dev libkf6coreaddons-dev libkf6kcmutils-dev`

## Installation

Run the provided installation script to install the effect for the current user:

```bash
chmod +x install.sh
./install.sh
```

After installation, the effect will be enabled automatically. You can find it and customize its settings in:
**System Settings -> Desktop Effects -> Mouse Trail**.

## Uninstallation

Run the provided uninstallation script:

```bash
chmod +x uninstall.sh
./uninstall.sh
```

## Credits

Inspired by various mouse trail effects and the KWin developer documentation.
Built with love for the KDE community.
