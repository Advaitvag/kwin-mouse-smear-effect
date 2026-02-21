# Mouse Smear KWin Effect

An aesthetically pleasing mouse cursor trail effect for KDE Plasma 6.

## Features

- **Customizable Trail**: Adjust color, decay rate (lifespan), and size.
- **Rainbow Mode**: Cycles through a spectrum of colors.
- **Sparkle Mode**: Adds sparkling stars to your cursor movement.
- **Gradient Mode**: Adds color variation to the trail.
- **Environmental Effects**: Gravity and Wander (drift) settings for a more dynamic look.
- **Pure QML**: High performance using `QtQuick.Particles`.

## Requirements

- KDE Plasma 6.0 or higher.
- `kpackagetool6`, `kwriteconfig6`, and `qdbus6` (standard KDE utilities).

## Installation

Run the provided installation script to install the effect for the current user:

```bash
chmod +x install.sh
./install.sh
```

After installation, the effect will be enabled automatically. You can find it and customize its settings in:
**System Settings -> Desktop Effects -> Mouse Smear**.

## Uninstallation

Run the provided uninstallation script:

```bash
chmod +x uninstall.sh
./uninstall.sh
```

## Credits

Inspired by various mouse trail effects and the KWin developer documentation.
Built with love for the KDE community.
