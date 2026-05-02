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
    enable_language(OBJC OBJCXX)
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
if(TARGET_PLATFORM_DARWIN)
    configure_target(
        TARGETS CRYSTAL_SHARED CRYSTAL_STATIC
        SOURCES
            src/modules/crystal/CRYSTAL.darwin.mm
        DEPENDS
            "-framework Cocoa"
    )
elseif(TARGET_PLATFORM_WIN32)
    configure_target(
        TARGETS CRYSTAL_SHARED CRYSTAL_STATIC
        SOURCES
            src/modules/crystal/CRYSTAL.win32.cpp
    )
else()
    message(WARNING "No platform-specific source files configured for CRYSTAL!")
endif()

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
            if(TARGET_PLATFORM_WIN32 AND TEST_NAME STREQUAL "exe")
                set_target_properties(${TEST_NAME} PROPERTIES
                    WIN32_EXECUTABLE TRUE
                )
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
