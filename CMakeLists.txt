cmake_minimum_required(VERSION 3.16)

project(kwin-mouse-smear)

set(QT_MIN_VERSION "6.6.0")
set(KF_MIN_VERSION "6.0.0")

find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(FeatureSummary)
include(ECMQtDeclareLoggingCategory)

find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS Core Gui Quick Widgets Svg DBus)
find_package(KF6 ${KF_MIN_VERSION} REQUIRED COMPONENTS Config ConfigWidgets GlobalAccel I18n CoreAddons KCMUtils)
find_package(KWin REQUIRED)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(smear_SOURCES
    src/main.cpp
    src/smear.cpp
)

kconfig_add_kcfg_files(smear_SOURCES
    src/smearconfig.kcfgc
)

add_library(mouse-smear MODULE ${smear_SOURCES})

target_link_libraries(mouse-smear PRIVATE
    KWin::kwin
    KF6::ConfigGui
    KF6::GlobalAccel
    KF6::I18n
    Qt6::Core
    Qt6::Gui
    Qt6::OpenGL
    Qt6::Svg
)

target_include_directories(mouse-smear PRIVATE ${KWin_INCLUDE_DIRS})

# KCM
set(mouse-smear_config_SRCS src/smear_config.cpp)
ki18n_wrap_ui(mouse-smear_config_SRCS src/smear_config.ui)
kconfig_add_kcfg_files(mouse-smear_config_SRCS src/smearconfig.kcfgc)

add_library(mouse-smear-config MODULE ${mouse-smear_config_SRCS})
target_link_libraries(mouse-smear-config
    KF6::KCMUtils
    KF6::CoreAddons
    KF6::I18n
    Qt6::Widgets
    Qt6::DBus
)

install(TARGETS mouse-smear DESTINATION ${KDE_INSTALL_PLUGINDIR}/kwin/effects/plugins)
install(TARGETS mouse-smear-config DESTINATION ${KDE_INSTALL_PLUGINDIR}/kwin/effects/configs)
install(FILES metadata.json DESTINATION ${KDE_INSTALL_DATADIR}/kwin/effects/mouse-smear)
install(FILES src/smear.kcfg DESTINATION ${KDE_INSTALL_DATADIR}/kwin/effects/mouse-smear/contents/config RENAME main.xml)
install(DIRECTORY package/contents/assets DESTINATION ${KDE_INSTALL_DATADIR}/kwin/effects/mouse-smear/contents)
