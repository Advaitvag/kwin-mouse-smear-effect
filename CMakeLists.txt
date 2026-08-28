cmake_minimum_required(VERSION 3.16)

project(kwin-mouse-trail)

set(QT_MIN_VERSION "6.6.0")
set(KF_MIN_VERSION "6.0.0")
# KWin's own CMake config (find_package(KWin)) hard-requires ECM >= 6.26.0,
# so asking for anything older here only produces a confusing later error.
set(ECM_MIN_VERSION "6.26.0")

find_package(ECM ${ECM_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(FeatureSummary)

find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS Core Gui Quick Widgets DBus)
find_package(KF6 ${KF_MIN_VERSION} REQUIRED COMPONENTS Config ConfigWidgets GlobalAccel I18n CoreAddons KCMUtils)
find_package(KWin REQUIRED)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# KWin exposes NO stable plugin ABI. Every KWin release may change the vtable
# layout / mangled signatures of KWin::Effect (e.g. 6.7 dropped the
# std::chrono::milliseconds presentTime argument from prePaintScreen() and
# prePaintWindow(), and removed windowInputMouseEvent()). A binary effect built
# against a different KWin version fails to dlopen with "undefined symbol" and
# silently never appears -- so record what we built against and let install.sh
# detect a stale build after a KWin upgrade.
if(NOT KWin_VERSION)
    set(KWin_VERSION "unknown")
endif()
message(STATUS "Building against KWin ${KWin_VERSION} (binary effects must match the running KWin)")
file(WRITE ${CMAKE_BINARY_DIR}/built-against-kwin.txt "${KWin_VERSION}\n")

set(trail_SOURCES
    src/main.cpp
    src/trail.cpp
)

kconfig_add_kcfg_files(trail_SOURCES
    src/trailconfig.kcfgc
)

add_library(mouse-trail MODULE ${trail_SOURCES})

target_link_libraries(mouse-trail PRIVATE
    KWin::kwin
    KF6::ConfigGui
    KF6::GlobalAccel
    KF6::I18n
    Qt6::Core
    Qt6::Gui
    Qt6::OpenGL
)

target_include_directories(mouse-trail PRIVATE ${KWin_INCLUDE_DIRS})

# KCM
set(mouse-trail_config_SRCS src/trail_config.cpp)
ki18n_wrap_ui(mouse-trail_config_SRCS src/trail_config.ui)
kconfig_add_kcfg_files(mouse-trail_config_SRCS src/trailconfig.kcfgc)

add_library(mouse-trail-config MODULE ${mouse-trail_config_SRCS})
target_link_libraries(mouse-trail-config
    KF6::KCMUtils
    KF6::CoreAddons
    KF6::I18n
    Qt6::Widgets
    Qt6::DBus
)

install(TARGETS mouse-trail DESTINATION ${KDE_INSTALL_PLUGINDIR}/kwin/effects/plugins)
install(TARGETS mouse-trail-config DESTINATION ${KDE_INSTALL_PLUGINDIR}/kwin/effects/configs)

# NOTE: deliberately NO install of metadata.json / trail.kcfg into
# ${KDE_INSTALL_DATADIR}/kwin/effects/mouse-trail/. That directory layout is
# KPackage's, and is only for *scripted* (QML/JS) effects. For a binary effect
# the metadata is already embedded in mouse-trail.so by
# KWIN_EFFECT_FACTORY_SUPPORTED_ENABLED(..., "../metadata.json", ...), and the
# KCM reads its schema from the compiled-in TrailConfig class. Installing it
# anyway made KWin log
#   KPackageStructure of KPluginMetaData(...) does not match requested format "KWin/Effect"
# on every single effect scan.
