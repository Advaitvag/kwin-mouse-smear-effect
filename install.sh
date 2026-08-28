#!/bin/bash
#
# KWin Mouse Trail Effect installer.
#
# Usage:
#   ./install.sh          build, install, enable and verify the effect
#   ./install.sh --check  don't build anything, just diagnose the current state
#
# Why this script is paranoid: KWin has no stable plugin ABI. After any KWin
# upgrade the previously installed mouse-trail.so can stop resolving against
# libkwin.so, and KWin then just skips it -- no error dialog, the effect simply
# vanishes from Desktop Effects. So we (1) rebuild from scratch whenever the
# KWin version changed, (2) refuse to install a .so with unresolved symbols,
# and (3) verify over D-Bus that KWin really loaded it.

set -uo pipefail

EFFECT_ID="mouse-trail"
REPO_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$REPO_DIR/build"
STAMP="$BUILD_DIR/built-against-kwin.txt"

die() { printf '\nERROR: %s\n' "$*" >&2; exit 1; }
info() { printf '\n==> %s\n' "$*"; }

# --- locate a qdbus binary (Plasma 6 ships qdbus6; some distros only 'qdbus') -
QDBUS=""
for c in qdbus6 qdbus-qt6 qdbus; do
    if command -v "$c" >/dev/null 2>&1; then QDBUS="$c"; break; fi
done

kwin_running_version() {
    # e.g. "kwin 6.7.4" -> "6.7.4"
    local v
    v="$(kwin_wayland --version 2>/dev/null || kwin_x11 --version 2>/dev/null || true)"
    printf '%s' "${v##* }"
}

installed_plugin_path() {
    local p
    for p in /usr/lib/qt6/plugins/kwin/effects/plugins/mouse-trail.so \
             /usr/lib/x86_64-linux-gnu/qt6/plugins/kwin/effects/plugins/mouse-trail.so \
             /usr/lib64/qt6/plugins/kwin/effects/plugins/mouse-trail.so; do
        [ -f "$p" ] && { printf '%s' "$p"; return 0; }
    done
    return 1
}

# Unresolved symbols against the *current* libkwin are the exact symptom of a
# plugin built for another KWin version.
abi_errors() {
    local so="$1"
    ldd -r "$so" 2>&1 | grep -E 'undefined symbol|not found'
}

diagnose() {
    local so kv loaded
    kv="$(kwin_running_version)"
    info "Diagnostics"
    echo "Running KWin:        ${kv:-unknown}"
    echo "Built against KWin:  $( [ -f "$STAMP" ] && cat "$STAMP" || echo 'no build stamp' )"

    if so="$(installed_plugin_path)"; then
        echo "Installed plugin:    $so"
        local errs
        errs="$(abi_errors "$so")"
        if [ -n "$errs" ]; then
            echo "ABI check:           FAILED -- plugin does not match this KWin:"
            printf '%s\n' "$errs" | sed 's/^/                       /'
            echo "                     -> re-run ./install.sh to rebuild against KWin ${kv}"
        else
            echo "ABI check:           ok (all symbols resolve against libkwin)"
        fi
    else
        echo "Installed plugin:    NOT INSTALLED"
    fi

    echo "kwinrc enabled:      $(kreadconfig6 --file kwinrc --group Plugins --key "${EFFECT_ID}Enabled" 2>/dev/null || echo '?')"
    if [ -n "$QDBUS" ]; then
        loaded="$($QDBUS org.kde.KWin /Effects org.kde.kwin.Effects.isEffectLoaded "$EFFECT_ID" 2>/dev/null)"
        echo "KWin isEffectLoaded: ${loaded:-<no reply>}"
        echo "KWin isEffectSupported: $($QDBUS org.kde.KWin /Effects org.kde.kwin.Effects.isEffectSupported "$EFFECT_ID" 2>/dev/null)"
    else
        echo "KWin D-Bus:          qdbus6 not found, cannot query"
    fi
    echo
    echo "KWin's own log lines for this effect:"
    journalctl --user -b -o cat 2>/dev/null | grep -iE 'mouse-trail|MouseTrail' | tail -5 | sed 's/^/  /' \
        || echo "  (none)"
}

if [ "${1:-}" = "--check" ]; then
    diagnose
    exit 0
fi

command -v cmake >/dev/null 2>&1 || die "cmake not found. Install cmake first."
command -v make  >/dev/null 2>&1 || die "make not found. Install base build tools first."

# --- 1. Force a clean rebuild when the KWin version changed -------------------
KWIN_VERSION="$(kwin_running_version)"
if [ -d "$BUILD_DIR" ]; then
    if [ ! -f "$STAMP" ] || [ "$(cat "$STAMP")" != "$KWIN_VERSION" ]; then
        info "KWin version changed (build stamp: $( [ -f "$STAMP" ] && cat "$STAMP" || echo none ), running: ${KWIN_VERSION:-unknown}) -- wiping stale build dir"
        rm -rf "$BUILD_DIR"
    fi
fi

# --- 2. Configure ------------------------------------------------------------
info "Configuring (KWin ${KWIN_VERSION:-unknown})"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR" || die "cannot enter $BUILD_DIR"

if ! cmake .. -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release 2>&1 | tee cmake.log; then
    if grep -q 'provided by "ECM"' cmake.log; then
        die "Extra CMake Modules (ECM) is missing. Install it:
    Arch/CachyOS : sudo pacman -S --needed extra-cmake-modules
    Fedora       : sudo dnf install extra-cmake-modules
    Debian/Neon  : sudo apt install extra-cmake-modules"
    fi
    if grep -q 'provided by "KWin"' cmake.log; then
        die "KWin development files are missing. Install them:
    Arch/CachyOS : sudo pacman -S --needed kwin      (headers ship in the kwin package)
    Fedora       : sudo dnf install kwin-devel
    Debian/Neon  : sudo apt install kwin-dev"
    fi
    die "CMake configuration failed. See $BUILD_DIR/cmake.log"
fi

# --- 3. Build ----------------------------------------------------------------
info "Building"
make -j"$(nproc)" || die "Build failed."

BUILT_SO="$(find "$BUILD_DIR" -name 'mouse-trail.so' -print -quit)"
[ -n "$BUILT_SO" ] || die "Build reported success but mouse-trail.so was not produced."

# --- 4. Verify the fresh binary matches this KWin BEFORE installing it --------
info "Checking the built plugin against the running KWin"
ABI="$(abi_errors "$BUILT_SO")"
if [ -n "$ABI" ]; then
    printf '%s\n' "$ABI" >&2
    die "The freshly built plugin has unresolved symbols against libkwin.
This means the KWin C++ effect API changed and src/ needs updating for KWin ${KWIN_VERSION}."
fi
echo "ok: all symbols resolve against libkwin"

# --- 5. Install --------------------------------------------------------------
info "Installing plugin (sudo required, installs under /usr)"
sudo make install || die "make install failed."

# Older versions of this project installed a KPackage-style directory for what
# is actually a binary effect; KWin logs a KPackageStructure mismatch for every
# effect scan as long as it exists. Clean it up.
for stale in /usr/share/kwin/effects/mouse-trail \
             /usr/share/kwin/effects/mouse-smear \
             /usr/share/kwin/effects/mouse_smear \
             /usr/share/kwin/effects/kwin_mouse_smear \
             /usr/share/kwin/effects/kwin_mouse_smear.json; do
    if [ -e "$stale" ]; then
        info "Removing stale leftover: $stale"
        sudo rm -rf "$stale"
    fi
done

# --- 6. Enable + force KWin to re-dlopen the new binary -----------------------
info "Enabling effect in kwinrc"
kwriteconfig6 --file kwinrc --group Plugins --key "${EFFECT_ID}Enabled" true
# stale ids from earlier versions of this project, so they can't fight with us
for old in mouse_trail mouse-smear mouse_smear kwin_mouse_smear smear_mouse_trail; do
    kwriteconfig6 --file kwinrc --group Plugins --key "${old}Enabled" --delete 2>/dev/null
done

if [ -n "$QDBUS" ]; then
    info "Reloading KWin"
    $QDBUS org.kde.KWin /KWin org.kde.KWin.reconfigure >/dev/null 2>&1
    # KWin keeps the previous .so mapped, so a plain reconfigure will happily
    # keep running the OLD code (this is why the effect used to need toggling
    # off/on by hand). Unload first, then load the new file.
    $QDBUS org.kde.KWin /Effects org.kde.kwin.Effects.unloadEffect "$EFFECT_ID" >/dev/null 2>&1
    sleep 0.5
    $QDBUS org.kde.KWin /Effects org.kde.kwin.Effects.loadEffect "$EFFECT_ID" >/dev/null 2>&1
    sleep 0.5

    LOADED="$($QDBUS org.kde.KWin /Effects org.kde.kwin.Effects.isEffectLoaded "$EFFECT_ID" 2>/dev/null)"
    if [ "$LOADED" = "true" ]; then
        info "Done -- KWin reports the Mouse Trail effect is loaded and running."
        echo "Customize it in System Settings -> Desktop Effects -> Mouse Trail."
        exit 0
    fi

    printf '\nInstalled, but KWin has not loaded the effect yet.\n'
    diagnose
    printf '\nIf the ABI check above is ok, log out and back in (on Wayland KWin cannot\nalways swap a plugin binary in a live session), then run ./install.sh --check\n'
    exit 1
else
    info "Installed. qdbus6 not found, so the effect could not be (re)loaded automatically."
    echo "Log out and back in, then run ./install.sh --check"
fi
