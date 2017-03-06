cmake_minimum_required (VERSION 2.6)
project (ScreenToolsSysIndicator)

set (PROJECT_VERSION_MAJOR 1)
set (PROJECT_VERSION_MINOR 0)

set (SHORT_NAME screentools)
set (DBUS_NAME com.canonical.indicator.${SHORT_NAME})
set (DBUS_PATH /com/canonical/indicator/${SHORT_NAME})

if(NOT DEFINED DISPLAY_ON_GREETER_AND_LOCK)
	set(GREETLOCKCOMMENT "#")
else()
	set(DISPLAY_ON_GREETER_AND_LOCK 1)
endif(NOT DEFINED DISPLAY_ON_GREETER_AND_LOCK)

include(GNUInstallDirs)
include_directories("${PROJECT_BINARY_DIR}")

configure_file (
  "${PROJECT_SOURCE_DIR}/config.h.in"
  "${PROJECT_BINARY_DIR}/config.h"
  )

configure_file (
  "${PROJECT_SOURCE_DIR}/data/unityIndicatorDef.in"
  "${PROJECT_BINARY_DIR}/data/${DBUS_NAME}"
  )
  
configure_file (
  "${PROJECT_SOURCE_DIR}/data/indicatorDbusServiceDef.in"
  "${PROJECT_BINARY_DIR}/data/${DBUS_NAME}.service"
  )

if(DEFINED DISPLAY_ON_GREETER_AND_LOCK)
	configure_file (
	  "${PROJECT_SOURCE_DIR}/data/90_unity-greeter.gschema.override.in"
	  "${PROJECT_BINARY_DIR}/data/90_unity-greeter.gschema.override"
	  )
endif(DEFINED DISPLAY_ON_GREETER_AND_LOCK)

find_package (PkgConfig REQUIRED)
pkg_check_modules (GIO2 REQUIRED gio-2.0)
include_directories (${GIO2_INCLUDE_DIRS})
link_directories (${GIO2_LIBRARY_DIRS})
add_definitions (${GIO2_CFLAGS_OTHER})

add_executable (indicator-${SHORT_NAME}-service src/indicator-${SHORT_NAME}-service.c)
target_link_libraries (indicator-${SHORT_NAME}-service ${GIO2_LIBRARIES})

#install (TARGETS indicator-${SHORT_NAME}-service DESTINATION ${CMAKE_INSTALL_LIBDIR}/${CMAKE_LIBRARY_ARCHITECTURE}/indicator-${SHORT_NAME})
install (TARGETS indicator-${SHORT_NAME}-service DESTINATION ${CMAKE_INSTALL_BINDIR})
install (DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/data/ DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons
          FILES_MATCHING PATTERN "*.svg"  PATTERN "*.png")
install (FILES ${PROJECT_BINARY_DIR}/data/${DBUS_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/unity/indicators)
#install (FILES ${PROJECT_BINARY_DIR}/data/${DBUS_NAME}.service DESTINATION ${CMAKE_INSTALL_PREFIX}/share/dbus-1/services)

#if(DEFINED DISPLAY_ON_GREETER_AND_LOCK)
#	install (FILES ${PROJECT_BINARY_DIR}/data/90_unity-greeter.gschema.override DESTINATION ${CMAKE_INSTALL_PREFIX}/share/glib-2.0/schemas)
#	install(SCRIPT post_install.cmake)
#endif(DEFINED DISPLAY_ON_GREETER_AND_LOCK)
