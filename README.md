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

## Troubleshooting

### "It just stopped working after a system update"

This is expected, and it is the single most likely failure. **KWin has no stable
plugin ABI.** A binary effect is compiled against the exact `libkwin.so` of one
KWin release; when KWin updates, the previously installed `mouse-trail.so` can
stop resolving and KWin silently skips it — no error, the effect simply
disappears from Desktop Effects.

For example KWin 6.7 changed `KWin::Effect` like this:

| KWin ≤ 6.6 | KWin ≥ 6.7 |
|---|---|
| `prePaintScreen(ScreenPrePaintData&, std::chrono::milliseconds)` | `prePaintScreen(ScreenPrePaintData&)` |
| `prePaintWindow(EffectWindow*, WindowPrePaintData&, std::chrono::milliseconds)` | `prePaintWindow(RenderView*, EffectWindow*, WindowPrePaintData&)` |
| `windowInputMouseEvent(QEvent*)` | *removed* |

Those symbols live in the plugin's vtable even when the effect doesn't override
them, so an old build dies at `dlopen` with `undefined symbol: _ZN4KWin6Effect...`.

**Fix:** re-run `./install.sh`. It detects that the KWin version changed, wipes
the stale build directory, rebuilds, and refuses to install a plugin whose
symbols don't resolve against the running KWin.

### Diagnosing the current state

```bash
./install.sh --check
```

This reports the running KWin version, the version the plugin was built against,
whether the installed `.so` resolves against `libkwin` (`ldd -r`), whether it is
enabled in `kwinrc`, and whether KWin actually has it loaded. The equivalent
manual checks:

```bash
ldd -r /usr/lib/qt6/plugins/kwin/effects/plugins/mouse-trail.so | grep undefined
qdbus6 org.kde.KWin /Effects org.kde.kwin.Effects.isEffectLoaded mouse-trail
journalctl --user -b -o cat | grep -i mouse-trail
```

### After rebuilding, KWin still runs the old code

KWin keeps the previous `.so` mapped, so a plain `reconfigure` is not enough
(this is why the effect used to need toggling off and on by hand). `install.sh`
now issues `unloadEffect` → `loadEffect` to force a fresh `dlopen`. If it still
won't load in a Wayland session, log out and back in, then `./install.sh --check`.

## Uninstallation

Run the provided uninstallation script:

```bash
chmod +x uninstall.sh
./uninstall.sh
```

## Credits

Inspired by various mouse trail effects and the KWin developer documentation.
Built with love for the KDE community.
