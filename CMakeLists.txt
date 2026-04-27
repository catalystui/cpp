cmake_minimum_required(VERSION 4.3.0)
project(CATLIB C CXX)
include_directories("${CMAKE_SOURCE_DIR}/include")
include(${CMAKE_SOURCE_DIR}/CMakeInit.cmake REQUIRED)
metadata("CATLIB" 1 0 0 "alpha.1" "Libraries, frameworks, and official utilities based on the CatalystUI Model written in C and C++.")
configure("${TARGET_VENDOR}" "${TARGET_SYSTEM}" "${TARGET_ARCHITECTURE}")
define()

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

# CRYSTAL
add_library(CRYSTAL SHARED
	src/modules/crystal/CRYSTAL.cpp
)
set_target_properties(CRYSTAL PROPERTIES PREFIX "")
