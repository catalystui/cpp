cmake_minimum_required(VERSION 4.3.0)
project(CATLIB C CXX)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
include_directories("${CMAKE_SOURCE_DIR}/include")
include(${CMAKE_SOURCE_DIR}/CMakeInit.cmake REQUIRED)
metadata("CATLIB" 1 0 0 "alpha.1" "Libraries, frameworks, and official utilities based on the CatalystUI Model written in C and C++.")
configure("${TARGET_VENDOR}" "${TARGET_SYSTEM}" "${TARGET_ARCHITECTURE}")
define("CMakeConfig.h" "CMAKECFG.h")

# DARWIN platform specifics
if(TARGET_PLATFORM_DARWIN)
    # Enable Objective-C and Objective-C++
    enable_language(OBJC OBJCXX)
elseif(TARGET_PLATFORM_LINUX)
    # Enable X11 support
    find_package(X11 REQUIRED)

    # Enable Wayland support
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(WAYLAND_CLIENT REQUIRED IMPORTED_TARGET wayland-client)
    pkg_check_modules(WAYLAND_PROTOCOLS REQUIRED wayland-protocols)
    pkg_get_variable(WAYLAND_PROTOCOLS_DIR wayland-protocols pkgdatadir)
    define_wayland_protocol(
        "${WAYLAND_PROTOCOLS_DIR}/stable/xdg-shell/xdg-shell.xml"
        "xdg-shell-client-protocol.h"
        "xdg-shell-client-protocol.c"
    )
    set(WAYLAND_XDG_SHELL_HEADER "${WAYLAND_PROTOCOL_HEADER}")
    set(WAYLAND_XDG_SHELL_SOURCE "${WAYLAND_PROTOCOL_SOURCE}")
    define_wayland_protocol(
        "${WAYLAND_PROTOCOLS_DIR}/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml"
        "xdg-decoration-unstable-v1-client-protocol.h"
        "xdg-decoration-unstable-v1-client-protocol.c"
    )
    set(WAYLAND_XDG_DECORATION_HEADER "${WAYLAND_PROTOCOL_HEADER}")
    set(WAYLAND_XDG_DECORATION_SOURCE "${WAYLAND_PROTOCOL_SOURCE}")
endif()

#######################
# Configure libraries #
#######################
# CATMMLIB
add_library(CATMMLIB_SHARED SHARED)
add_library(CATMMLIB_STATIC STATIC)
configure_target(
    NAME CATMMLIB
    TARGETS CATMMLIB_SHARED CATMMLIB_STATIC
    SOURCES
        src/CATMMLIB.cpp
)
if (TARGET_PLATFORM_DARWIN)
    configure_target(
        TARGETS CATMMLIB_SHARED CATMMLIB_STATIC
        SOURCES
            src/CATMMLIB.darwin.mm
    )
elseif(TARGET_PLATFORM_WIN32)
    configure_target(
        TARGETS CATMMLIB_SHARED CATMMLIB_STATIC
        SOURCES
            src/CATMMLIB.win32.cpp
    )
elseif(TARGET_PLATFORM_LINUX)
    configure_target(
        TARGETS CATMMLIB_SHARED CATMMLIB_STATIC
        SOURCES
            src/CATMMLIB.linux.cpp
    )
else()
    message(WARNING "No platform-specific source files configured for CATMMLIB!")
endif()

# CATTELIB
add_library(CATTELIB_SHARED SHARED)
add_library(CATTELIB_STATIC STATIC)
configure_target(
    NAME CATTELIB
    TARGETS CATTELIB_SHARED CATTELIB_STATIC
    SOURCES
        src/CATTELIB.cpp
)

# CRYSTAL
add_library(CRYSTAL_SHARED SHARED)
add_library(CRYSTAL_STATIC STATIC)
configure_target(
    NAME CRYSTAL
    TARGETS CRYSTAL_SHARED CRYSTAL_STATIC
    SOURCES
        src/modules/crystal/CRYSTAL.cpp
    DEPENDS
        CATMMLIB_SHARED
        CATTELIB_SHARED
)
# if(TARGET_PLATFORM_DARWIN)
#     configure_target(
#         TARGETS CRYSTAL_SHARED CRYSTAL_STATIC
#         SOURCES
#             src/modules/crystal/CRYSTAL.darwin.mm
#         DEPENDS
#             "-framework Cocoa"
#     )
# elseif(TARGET_PLATFORM_WIN32)
#     configure_target(
#         TARGETS CRYSTAL_SHARED CRYSTAL_STATIC
#         SOURCES
#             src/modules/crystal/CRYSTAL.win32.cpp
#     )
# elseif(TARGET_PLATFORM_LINUX)
#     add_library(CRYSTAL_X11_SHARED SHARED)
#     add_library(CRYSTAL_X11_STATIC STATIC)
#     configure_target(
#         NAME CRYSTAL-x11
#         TARGETS CRYSTAL_X11_SHARED CRYSTAL_X11_STATIC
#         SOURCES
#             src/modules/crystal/CRYSTAL.linux.x11.cpp
#         DEPENDS
#             CATMMLIB_SHARED
#             CATTELIB_SHARED
#             "X11::X11"
#     )

#     add_library(CRYSTAL_WAYLAND_SHARED SHARED)
#     add_library(CRYSTAL_WAYLAND_STATIC STATIC)
#     configure_target(
#         NAME CRYSTAL-wayland
#         TARGETS CRYSTAL_WAYLAND_SHARED CRYSTAL_WAYLAND_STATIC
#         SOURCES
#             src/modules/crystal/CRYSTAL.linux.wayland.cpp
#             "${WAYLAND_XDG_SHELL_SOURCE}"
#             "${WAYLAND_XDG_SHELL_HEADER}"
#             "${WAYLAND_XDG_DECORATION_SOURCE}"
#             "${WAYLAND_XDG_DECORATION_HEADER}"
#         DEPENDS
#             CATMMLIB_SHARED
#             CATTELIB_SHARED
#             "PkgConfig::WAYLAND_CLIENT"
#     )

#     configure_target(
#         TARGETS CRYSTAL_SHARED CRYSTAL_STATIC
#         SOURCES
#             src/modules/crystal/CRYSTAL.linux.cpp
#         DEPENDS
#             CRYSTAL_X11_SHARED
#             CRYSTAL_WAYLAND_SHARED
#     )
# else()
#     message(WARNING "No platform-specific source files configured for CRYSTAL!")
# endif()

# TEST
if(EXISTS "${CMAKE_SOURCE_DIR}/test")
    file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_SOURCE_DIR}/test/*.cpp"
    )
    if(TEST_SOURCES)
        message(STATUS "Test source files found, configuring test executables...")
        foreach(TEST_SOURCE ${TEST_SOURCES})
            get_filename_component(TEST_NAME "${TEST_SOURCE}" NAME_WE)
            add_executable(${TEST_NAME}
                "${TEST_SOURCE}"
            )
            if(TEST_NAME STREQUAL "exe")
                if(TARGET_PLATFORM_WIN32)
                    set_target_properties(${TEST_NAME} PROPERTIES
                        WIN32_EXECUTABLE TRUE
                    )
                    message(STATUS "Configured ${TEST_NAME} as a Windows GUI application")
                elseif(TARGET_PLATFORM_DARWIN)
                    set_target_properties(${TEST_NAME} PROPERTIES
                        MACOSX_BUNDLE TRUE
                        MACOSX_BUNDLE_BUNDLE_NAME "${TEST_NAME}"
                        MACOSX_BUNDLE_GUI_IDENTIFIER "org.catalystui.${TEST_NAME}"
                    )
                    message(STATUS "Configured ${TEST_NAME} as a macOS GUI application")
                elseif(TARGET_PLATFORM_LINUX)
                    # Also export a shared executable to test .so library loading
                    add_executable(${TEST_NAME}_shared
                        "${TEST_SOURCE}"
                    )
                    configure_target(
                        NAME ${TEST_NAME}_shared
                        FOLDER "test"
                        SUBDIR "test/"
                        TARGETS ${TEST_NAME}_shared
                        DEPENDS
                            CATMMLIB_SHARED
                            CATTELIB_SHARED
                            CRYSTAL_SHARED
                    )
                    message(STATUS "Configured ${TEST_NAME}_shared as a Linux GUI application")
                endif()
            endif()
            if(MSVC)
                target_link_options(${TEST_NAME} PRIVATE
                    /ENTRY:mainCRTStartup
                )
            endif()
            configure_target(
                NAME ${TEST_NAME}
                FOLDER "test"
                SUBDIR "test/"
                TARGETS ${TEST_NAME}
                DEPENDS
                    CATMMLIB_STATIC
                    CATTELIB_STATIC
                    CRYSTAL_STATIC
            )
        endforeach()
    else()
        message(STATUS "No .cpp test source files found, skipping test executable configuration")
        message(STATUS "    To configure test executables, add one or more .cpp files")
        message(STATUS "    to the 'test' directory, then reload the CMake configuration.")
    endif()
else()
    message(STATUS "No test directory found, skipping test executable configuration")
    message(STATUS "    To configure test executables, create a 'test' directory")
    message(STATUS "    in the project root and add one or more .cpp files,")
    message(STATUS "    then reload the CMake configuration.")
endif()
