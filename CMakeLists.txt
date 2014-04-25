project(recentfolders)

FIND_PACKAGE(Qt4 REQUIRED)
FIND_PACKAGE(KDE4 REQUIRED)
FIND_PACKAGE(Baloo REQUIRED)
set(CMAKE_INSTALL_PREFIX `kde4-config --prefix`)
add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(${KDE4_INCLUDES} ${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} . ${BALOO_INCLUDE_DIR})
 
set(recentfolders_PART_SRCS BalooQueryRecentFolders.cpp RecentFolders.cpp)
 
kde4_add_plugin(kio_recentfolders ${recentfolders_PART_SRCS})
 
target_link_libraries(kio_recentfolders ${KDE4_KIO_LIBS} ${BALOO_QUERY_LIBRARIES} ${BALOO_UTILS_LIBRARIES} ${BALOO_LIBRARIES})
 
install(TARGETS kio_recentfolders DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install(FILES recentfolders.protocol  DESTINATION  ${SERVICES_INSTALL_DIR})
