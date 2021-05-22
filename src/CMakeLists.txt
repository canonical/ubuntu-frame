cmake_minimum_required(VERSION 3.16)
project(ubuntu_core_kiosk)
include(FindPkgConfig)

set(CMAKE_CXX_STANDARD 20)
pkg_check_modules(MIRAL miral REQUIRED)
pkg_check_modules(WAYLAND_CLIENT REQUIRED wayland-client)

add_executable(ubuntu_core_kiosk
    ubuntu_core_kiosk_main.cpp
    kiosk_window_manager.cpp kiosk_window_manager.h
    egwallpaper.cpp egwallpaper.h
    egfullscreenclient.cpp egfullscreenclient.h
)

target_include_directories(ubuntu_core_kiosk PUBLIC SYSTEM ${MIRAL_INCLUDE_DIRS})
target_link_libraries(ubuntu_core_kiosk ${MIRAL_LDFLAGS} ${WAYLAND_CLIENT_LIBRARIES})

install(PROGRAMS ${CMAKE_BINARY_DIR}/ubuntu_core_kiosk
    DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
)
