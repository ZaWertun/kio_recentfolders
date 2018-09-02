cmake_minimum_required(VERSION 3.5)
set(QT_MIN_VERSION "5.4.0")
set(KF5_MIN_VERSION "5.16.0")

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    ${ECM_MODULE_PATH}
    ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMSetupVersion)
include(FeatureSummary)

find_package(Qt5 ${QT_MIN_VERSION} REQUIRED Core Network)
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED KIO I18n Baloo)
add_library(kio_recentfolders MODULE kio_recentfolders.cpp)

target_link_libraries(kio_recentfolders
    KF5::KIOCore
    KF5::I18n
    KF5::Baloo
    Qt5::Network)
set_target_properties(kio_recentfolders PROPERTIES OUTPUT_NAME "recentfolders")

install(TARGETS kio_recentfolders      DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf5/kio)
install(FILES   recentfolders.protocol DESTINATION ${SERVICES_INSTALL_DIR})
ki18n_install(po)
