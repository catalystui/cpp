cmake_minimum_required(VERSION 4.3.0)
project(CATLIB C CXX)
include_directories("${CMAKE_SOURCE_DIR}/include")
include(${CMAKE_SOURCE_DIR}/CMakeInit.cmake REQUIRED)
metadata("CATLIB" 1 0 0 "alpha.1" "Libraries, frameworks, and official utilities based on the CatalystUI Model written in C and C++.")
configure("${TARGET_VENDOR}" "${TARGET_SYSTEM}" "${TARGET_ARCHITECTURE}")
define()

# Enable Objective-C, Objective-C++ if applicable
if(TARGET_PLATFORM_DARWIN)
    enable_language(OBJC OBJCXX)
endif()

# Identify __STDC_VERSION__
if (CMAKE_C_STANDARD STREQUAL "95")
	set(CMAKECFG_STDC_VERSION 199409L)
elseif (CMAKE_C_STANDARD STREQUAL "99")
	set(CMAKECFG_STDC_VERSION 199901L)
elseif (CMAKE_C_STANDARD STREQUAL "11")
	set(CMAKECFG_STDC_VERSION 201112L)
elseif (CMAKE_C_STANDARD STREQUAL "17")
	set(CMAKECFG_STDC_VERSION 201710L)
elseif (CMAKE_C_STANDARD STREQUAL "23")
	set(CMAKECFG_STDC_VERSION 202311L)
else()
	set(CMAKECFG_STDC_VERSION 0)
endif()

# Check type sizes
include(CheckTypeSize)
check_type_size("void*" CMAKECFG_SIZEOF_VOID_P)

# Generate CMAKE header file
configure_file(
	"${CMAKE_SOURCE_DIR}/CMakeConfig.h"
	"${CMAKE_SOURCE_DIR}/target/generated/CMAKECFG.h"
	@ONLY
)
include_directories("${CMAKE_SOURCE_DIR}/target/generated")

# CATMMLIB
add_library(CATMMLIB SHARED
	src/CATMMLIB.cpp
)
add_library(CATMMLIB_STATIC STATIC
    src/CATMMLIB.cpp
)
set_target_properties(CATMMLIB PROPERTIES PREFIX "")

# CATTELIB
add_library(CATTELIB SHARED
    src/CATTELIB.cpp
)
add_library(CATTELIB_STATIC STATIC
    src/CATTELIB.cpp
)
set_target_properties(CATTELIB PROPERTIES PREFIX "")

# CRYSTAL
add_library(CRYSTAL SHARED
	src/modules/crystal/CRYSTAL.cpp
)
add_library(CRYSTAL_STATIC STATIC
    src/modules/crystal/CRYSTAL.cpp
)
set_target_properties(CRYSTAL PROPERTIES PREFIX "")
target_link_libraries(CRYSTAL PRIVATE
    CATMMLIB
)
if(TARGET_PLATFORM_DARWIN)
    target_sources(CRYSTAL PRIVATE
        src/modules/crystal/CRYSTAL.darwin.mm
    )
    target_link_libraries(CRYSTAL PRIVATE
        "-framework Cocoa"
    )
    target_sources(CRYSTAL_STATIC PRIVATE
        src/modules/crystal/CRYSTAL.darwin.mm
    )
    target_link_libraries(CRYSTAL_STATIC PRIVATE
        "-framework Cocoa"
    )
endif()

# TEST
if(EXISTS "${CMAKE_SOURCE_DIR}/test")
    file(GLOB TEST_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_SOURCE_DIR}/test/*.cpp"
    )
    if(TEST_SOURCES)
        message(STATUS "Test source files found, configuring test executables...")
        foreach(TEST_SOURCE ${TEST_SOURCES})
            get_filename_component(TEST_NAME "${TEST_SOURCE}" NAME_WE)
            add_executable(${TEST_NAME}
                "${TEST_SOURCE}"
            )
            if(TARGET_PLATFORM_WIN32)
                set_target_properties(${TEST_NAME} PROPERTIES
                    WIN32_EXECUTABLE TRUE
                )
            endif()
            if(MSVC)
                target_link_options(${TEST_NAME} PRIVATE
                    /ENTRY:mainCRTStartup
                )
            endif()
            add_dependencies(${TEST_NAME}
                CATMMLIB_STATIC
                CATTELIB_STATIC
                CRYSTAL_STATIC
            )
            target_link_libraries(${TEST_NAME} PRIVATE
                CATMMLIB_STATIC
                CATTELIB_STATIC
                CRYSTAL_STATIC
            )
			message(STATUS "Configured test executable ${TEST_NAME}")
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
