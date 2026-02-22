cmake_minimum_required(VERSION 3.16)

project(kwin-mouse-trail)

set(QT_MIN_VERSION "6.6.0")
set(KF_MIN_VERSION "6.0.0")

find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(FeatureSummary)
include(ECMQtDeclareLoggingCategory)

find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS Core Gui Quick Widgets DBus)
find_package(KF6 ${KF_MIN_VERSION} REQUIRED COMPONENTS Config ConfigWidgets GlobalAccel I18n CoreAddons KCMUtils)
find_package(KWin REQUIRED)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
install(FILES metadata.json DESTINATION ${KDE_INSTALL_DATADIR}/kwin/effects/mouse-trail)
install(FILES src/trail.kcfg DESTINATION ${KDE_INSTALL_DATADIR}/kwin/effects/mouse-trail/contents/config RENAME main.xml)
