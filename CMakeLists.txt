cmake_minimum_required(VERSION 3.15)
project(libnexrad VERSION 0.8.5 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Dependencies
find_package(BZip2 REQUIRED)
find_package(ZLIB REQUIRED)
find_library(MATH_LIBRARY m)

# Optional Dependencies for Rendering Adapters
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(CAIRO cairo)
endif()

# LVGL discovery (assuming installed or provided as a submodule/sysroot)
# We'll just check for the header for now or let the user provide the path
find_path(LVGL_INCLUDE_DIR lvgl.h)

# Options
option(NEXRAD_BUILD_EXAMPLES "Build example applications" ON)
option(NEXRAD_ENABLE_CAIRO "Enable Cairo rendering adapter" ${CAIRO_FOUND})
option(NEXRAD_ENABLE_LVGL "Enable LVGL rendering adapter" ${LVGL_INCLUDE_DIR})

# Configuration Header
include(CheckIncludeFile)
check_include_file("endian.h" HAVE_ENDIAN_H)
check_include_file("architecture/byte_order.h" HAVE_ARCH_BYTE_ORDER_H)

configure_file(cmake/config.h.in include/nexrad/config.h)

# Subdirectories
add_subdirectory(src)
if(NEXRAD_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
add_subdirectory(colors)

include(GNUInstallDirs)
install(EXPORT nexradTargets
    FILE nexradConfig.cmake
    NAMESPACE nexrad::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/nexrad
)
