execute_process(
    COMMAND git rev-parse --short=7 HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_HASH
    RESULT_VARIABLE GIT_RESULT
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)

function(metadata name version_major version_minor version_patch version_dash description)
    set(METADATA_NAME ${name})
    set(METADATA_VERSION_MAJOR ${version_major})
    set(METADATA_VERSION_MINOR ${version_minor})
    set(METADATA_VERSION_PATCH ${version_patch})
    if(NOT "${version_dash}" STREQUAL "")
        if (GIT_RESULT EQUAL 0)
            set(version_prerelease "-${version_dash}+${GIT_HASH}")
        else()
            set(version_prerelease "-${version_dash}")
        endif()
    endif()
    set(METADATA_VERSION ${version_major}.${version_minor}.${version_patch}${version_prerelease})
    set(METADATA_DESCRIPTION ${description})

    set(METADATA_NAME ${METADATA_NAME} PARENT_SCOPE)
    set(METADATA_VERSION_MAJOR ${METADATA_VERSION_MAJOR} PARENT_SCOPE)
    set(METADATA_VERSION_MINOR ${METADATA_VERSION_MINOR} PARENT_SCOPE)
    set(METADATA_VERSION_PATCH ${METADATA_VERSION_PATCH} PARENT_SCOPE)
    set(METADATA_VERSION ${METADATA_VERSION} PARENT_SCOPE)
    set(METADATA_DESCRIPTION ${METADATA_DESCRIPTION} PARENT_SCOPE)

    message(STATUS "Preparing build files for: ${METADATA_NAME} v${METADATA_VERSION}")
endfunction()

function(configure target_vendor target_system target_architecture)
    set(TARGET_VENDOR ${target_vendor} PARENT_SCOPE)
    set(TARGET_SYSTEM ${target_system} PARENT_SCOPE)
    set(TARGET_ARCHITECTURE ${target_architecture} PARENT_SCOPE)

    set(OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build/${target_vendor}/${target_system}")
    if(NOT "${target_architecture}" STREQUAL "")
        set(OUTPUT_DIRECTORY "${OUTPUT_DIRECTORY}/${target_architecture}")
    endif()
    message(STATUS "Setting output directory: ${OUTPUT_DIRECTORY}")
    file(REMOVE_RECURSE "${OUTPUT_DIRECTORY}")
    file(MAKE_DIRECTORY "${OUTPUT_DIRECTORY}")
    if(CMAKE_CONFIGURATION_TYPES)
        foreach(CONFIG_NAME ${CMAKE_CONFIGURATION_TYPES})
            string(TOLOWER "${CONFIG_NAME}" CONFIG_LOWER)
            if (CONFIG_LOWER STREQUAL "debug" OR CONFIG_LOWER STREQUAL "release")
                file(MAKE_DIRECTORY "${OUTPUT_DIRECTORY}/${CONFIG_LOWER}")
            endif()
        endforeach()
        set(OUTPUT_DIRECTORY "${OUTPUT_DIRECTORY}/$<LOWER_CASE:$<CONFIG>>")
    endif()
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIRECTORY}" PARENT_SCOPE)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_DIRECTORY}" PARENT_SCOPE)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_DIRECTORY}/lib" PARENT_SCOPE)

    # Identify target platform
    if(TARGET_VENDOR STREQUAL "microsoft" AND (TARGET_SYSTEM MATCHES "^nt" OR TARGET_SYSTEM MATCHES "^win"))
        set(CONFIGURE_FOUND_PLATFORM 1)
        set(TARGET_PLATFORM_WIN32 1 PARENT_SCOPE)
        set(TARGET_PLATFORM_WIN32 1)
        message(STATUS "Identified target platform as Win32 based on vendor '${TARGET_VENDOR}' and system '${TARGET_SYSTEM}'")
    else()
        set(TARGET_PLATFORM_WIN32 0 PARENT_SCOPE)
        set(TARGET_PLATFORM_WIN32 0)
        add_compile_definitions(TARGET_PLATFORM_WIN32=0)
    endif()
    if(TARGET_VENDOR STREQUAL "apple" AND TARGET_SYSTEM MATCHES "^darwin")
        set(CONFIGURE_FOUND_PLATFORM 1)
        set(TARGET_PLATFORM_DARWIN 1 PARENT_SCOPE)
        set(TARGET_PLATFORM_DARWIN 1)
        message(STATUS "Identified target platform as Darwin based on vendor '${TARGET_VENDOR}' and system '${TARGET_SYSTEM}'")
    else()
        set(TARGET_PLATFORM_DARWIN 0 PARENT_SCOPE)
        set(TARGET_PLATFORM_DARWIN 0)
    endif()
    if(TARGET_VENDOR STREQUAL "linux" AND TARGET_SYSTEM MATCHES "^linux")
        set(CONFIGURE_FOUND_PLATFORM 1)
        set(TARGET_PLATFORM_LINUX 1 PARENT_SCOPE)
        set(TARGET_PLATFORM_LINUX 1)
        message(STATUS "Identified target platform as Linux based on vendor '${TARGET_VENDOR}' and system '${TARGET_SYSTEM}'")
    else()
        set(TARGET_PLATFORM_LINUX 0 PARENT_SCOPE)
        set(TARGET_PLATFORM_LINUX 0)
    endif()
    if(NOT CONFIGURE_FOUND_PLATFORM)
        message(FATAL_ERROR "Unable to identify a target platform based on vendor '${TARGET_VENDOR}' and system '${TARGET_SYSTEM}'")
    endif()

    # Set library prefixes based on platform conventions
    if(TARGET_PLATFORM_DARWIN OR TARGET_PLATFORM_LINUX)
        set(CMAKE_SHARED_LIBRARY_PREFIX "lib" PARENT_SCOPE)
        set(CMAKE_STATIC_LIBRARY_PREFIX "lib" PARENT_SCOPE)
        message(STATUS "Using 'lib' prefix (UNIX-like conventions) for shared and static libraries")
    else(TARGET_PLATFORM_WIN32)
        set(CMAKE_SHARED_LIBRARY_PREFIX "" PARENT_SCOPE)
        set(CMAKE_STATIC_LIBRARY_PREFIX "" PARENT_SCOPE)
        message(STATUS "Stripped library prefixes for shared and static libraries")
    endif()

    # Identify UNICODE support
    if(TARGET_VENDOR STREQUAL "microsoft" AND (TARGET_SYSTEM MATCHES "^nt"))
        set(TARGET_SUPPORTS_UNICODE 1 PARENT_SCOPE)
    elseif(TARGET_VENDOR STREQUAL "apple" AND TARGET_SYSTEM MATCHES "^darwin")
        # TODO: Determine when the cutoff for UNICODE support on Darwin is, and condition this on the version
        set(TARGET_SUPPORTS_UNICODE 1 PARENT_SCOPE)
    elseif(TARGET_VENDOR STREQUAL "linux" AND TARGET_SYSTEM MATCHES "^linux")
        # TODO: Determine when the cutoff for UNICODE support on Linux is, and condition this on the version and/or C library
        set(TARGET_SUPPORTS_UNICODE 1 PARENT_SCOPE)
    else()
        set(TARGET_SUPPORTS_UNICODE 0 PARENT_SCOPE)
    endif()

    # Identify the STDC version
    if (CMAKE_C_STANDARD STREQUAL "95")
        set(TARGET_STDC_VERSION 199409L)
    elseif (CMAKE_C_STANDARD STREQUAL "99")
        set(TARGET_STDC_VERSION 199901L)
    elseif (CMAKE_C_STANDARD STREQUAL "11")
        set(TARGET_STDC_VERSION 201112L)
    elseif (CMAKE_C_STANDARD STREQUAL "17")
        set(TARGET_STDC_VERSION 201710L)
    elseif (CMAKE_C_STANDARD STREQUAL "23")
        set(TARGET_STDC_VERSION 202311L)
    else()
        set(TARGET_STDC_VERSION 0)
    endif()
    set(TARGET_STDC_VERSION ${TARGET_STDC_VERSION} PARENT_SCOPE)
    message(STATUS "Identified C standard version as ${CMAKE_C_STANDARD} (STDC version ${TARGET_STDC_VERSION})")

    # Determine the size of a pointer
    include(CheckTypeSize)
    check_type_size("void*" TARGET_SIZEOF_VOID_P)
    set(TARGET_SIZEOF_VOID_P ${TARGET_SIZEOF_VOID_P} PARENT_SCOPE)
    message(STATUS "Determined size of void* pointer: ${TARGET_SIZEOF_VOID_P} bytes")
endfunction()

function(configure_target)
    cmake_parse_arguments(ARG "" "NAME;FOLDER;SUBDIR" "TARGETS;SOURCES;DEPENDS" ${ARGN})
    set(ARG_SUBDIR_LOWER "")
    if(NOT "${ARG_SUBDIR}" STREQUAL "")
        string(TOLOWER "${ARG_SUBDIR}" ARG_SUBDIR_LOWER)
    endif()
    foreach(TARGET_NAME ${ARG_TARGETS})
        if(NOT "${ARG_NAME}" STREQUAL "")
            set(CONFIGURE_OUTPUT_NAME "${ARG_NAME}")
            if(TARGET_VENDOR STREQUAL "linux" AND TARGET_SYSTEM MATCHES "^linux")
                string(TOLOWER "${CONFIGURE_OUTPUT_NAME}" CONFIGURE_OUTPUT_NAME)
            endif()
            set_target_properties(${TARGET_NAME} PROPERTIES
                OUTPUT_NAME "${CONFIGURE_OUTPUT_NAME}"
            )
        endif()
        if(NOT "${ARG_FOLDER}" STREQUAL "")
            set_target_properties(${TARGET_NAME} PROPERTIES
                FOLDER "${ARG_FOLDER}"
            )
        endif()
        get_target_property(CONFIGURE_TARGET_TYPE ${TARGET_NAME} TYPE)
        if(NOT "${ARG_SUBDIR_LOWER}" STREQUAL "")
            if(CONFIGURE_TARGET_TYPE STREQUAL "EXECUTABLE")
                set_target_properties(${TARGET_NAME} PROPERTIES
                    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${ARG_SUBDIR_LOWER}"
                )
            elseif(CONFIGURE_TARGET_TYPE STREQUAL "STATIC_LIBRARY")
                set_target_properties(${TARGET_NAME} PROPERTIES
                    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/${ARG_SUBDIR_LOWER}"
                )
            elseif(CONFIGURE_TARGET_TYPE STREQUAL "SHARED_LIBRARY")
                if(WIN32)
                    set_target_properties(${TARGET_NAME} PROPERTIES
                        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${ARG_SUBDIR_LOWER}"
                        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/${ARG_SUBDIR_LOWER}"
                    )
                else()
                    set_target_properties(${TARGET_NAME} PROPERTIES
                        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${ARG_SUBDIR_LOWER}"
                    )
                endif()
            elseif(CONFIGURE_TARGET_TYPE STREQUAL "MODULE_LIBRARY")
                set_target_properties(${TARGET_NAME} PROPERTIES
                    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${ARG_SUBDIR_LOWER}"
                )
            endif()
        endif()

        if(ARG_SOURCES)
            target_sources(${TARGET_NAME} PRIVATE ${ARG_SOURCES})
        endif()

        if(ARG_DEPENDS)
            target_link_libraries(${TARGET_NAME} PRIVATE ${ARG_DEPENDS})
        endif()
    endforeach()

    # Report configuration details
    set(CONFIGURE_TARGET_MESSAGE "Configured target(s) ${ARG_TARGETS}")
    if(NOT "${ARG_NAME}" STREQUAL "")
        set(CONFIGURE_TARGET_MESSAGE "${CONFIGURE_TARGET_MESSAGE} with output name '${ARG_NAME}'")
    endif()
    if(NOT "${ARG_FOLDER}" STREQUAL "")
        set(CONFIGURE_TARGET_MESSAGE "${CONFIGURE_TARGET_MESSAGE}, folder '${ARG_FOLDER}'")
    endif()
    if(NOT "${ARG_SUBDIR_LOWER}" STREQUAL "")
        set(CONFIGURE_TARGET_MESSAGE "${CONFIGURE_TARGET_MESSAGE}, output subdir '${ARG_SUBDIR_LOWER}'")
    endif()
    if(ARG_SOURCES)
        set(CONFIGURE_TARGET_MESSAGE "${CONFIGURE_TARGET_MESSAGE}, sources: ${ARG_SOURCES}")
    endif()
    if(ARG_DEPENDS)
        set(CONFIGURE_TARGET_MESSAGE "${CONFIGURE_TARGET_MESSAGE}, dependencies: ${ARG_DEPENDS}")
    endif()
    message(STATUS "${CONFIGURE_TARGET_MESSAGE}")
endfunction()

function(define config_file_name config_header_name)
    # Add compile definitions
    add_compile_definitions(
        METADATA_NAME=\"${METADATA_NAME}\"
        METADATA_VERSION_MAJOR=${METADATA_VERSION_MAJOR}
        METADATA_VERSION_MINOR=${METADATA_VERSION_MINOR}
        METADATA_VERSION_PATCH=${METADATA_VERSION_PATCH}
        METADATA_VERSION=\"${METADATA_VERSION}\"
        METADATA_DESCRIPTION=\"${METADATA_DESCRIPTION}\"
        TARGET_VENDOR=\"${TARGET_VENDOR}\"
        TARGET_SYSTEM=\"${TARGET_SYSTEM}\"
        TARGET_ARCHITECTURE=\"${TARGET_ARCHITECTURE}\"
        TARGET_PLATFORM_WIN32=${TARGET_PLATFORM_WIN32}
        TARGET_PLATFORM_DARWIN=${TARGET_PLATFORM_DARWIN}
        TARGET_PLATFORM_LINUX=${TARGET_PLATFORM_LINUX}
        TARGET_STDC_VERSION=${TARGET_STDC_VERSION}
        TARGET_SIZEOF_VOID_P=${TARGET_SIZEOF_VOID_P}
        TARGET_SUPPORTS_UNICODE=${TARGET_SUPPORTS_UNICODE}
        "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:TARGET_SHARED=1>"
    )
    message(STATUS "Added compile definition METADATA_NAME=\"${METADATA_NAME}\"")
    message(STATUS "Added compile definition METADATA_VERSION_MAJOR=${METADATA_VERSION_MAJOR}")
    message(STATUS "Added compile definition METADATA_VERSION_MINOR=${METADATA_VERSION_MINOR}")
    message(STATUS "Added compile definition METADATA_VERSION_PATCH=${METADATA_VERSION_PATCH}")
    message(STATUS "Added compile definition METADATA_VERSION=\"${METADATA_VERSION}\"")
    message(STATUS "Added compile definition METADATA_DESCRIPTION=\"${METADATA_DESCRIPTION}\"")
    message(STATUS "Added compile definition TARGET_VENDOR=\"${TARGET_VENDOR}\"")
    message(STATUS "Added compile definition TARGET_SYSTEM=\"${TARGET_SYSTEM}\"")
    message(STATUS "Added compile definition TARGET_ARCHITECTURE=\"${TARGET_ARCHITECTURE}\"")
    message(STATUS "Added compile definition TARGET_PLATFORM_WIN32=${TARGET_PLATFORM_WIN32}")
    message(STATUS "Added compile definition TARGET_PLATFORM_DARWIN=${TARGET_PLATFORM_DARWIN}")
    message(STATUS "Added compile definition TARGET_PLATFORM_LINUX=${TARGET_PLATFORM_LINUX}")
    message(STATUS "Added compile definition TARGET_STDC_VERSION=${TARGET_STDC_VERSION}")
    message(STATUS "Added compile definition TARGET_SIZEOF_VOID_P=${TARGET_SIZEOF_VOID_P}")
    message(STATUS "Added compile definition TARGET_SUPPORTS_UNICODE=${TARGET_SUPPORTS_UNICODE}")
    message(STATUS "Added compile definition TARGET_SHARED=1 for shared library targets")

    # Process the configuration file/header
    if(EXISTS "${CMAKE_SOURCE_DIR}/${config_file_name}")
        set(GENERATED_REAL_DIR "${CMAKE_BINARY_DIR}/generated")
        set(GENERATED_REAL_FILE "${GENERATED_REAL_DIR}/${config_header_name}")
        get_filename_component(BINARY_PARENT_DIR "${CMAKE_BINARY_DIR}" DIRECTORY)
        set(GENERATED_LINK_DIR "${BINARY_PARENT_DIR}/generated")
        set(GENERATED_LINK_FILE "${GENERATED_LINK_DIR}/${config_header_name}")
        file(MAKE_DIRECTORY "${GENERATED_REAL_DIR}")
        file(MAKE_DIRECTORY "${GENERATED_LINK_DIR}")
        configure_file(
            "${CMAKE_SOURCE_DIR}/${config_file_name}"
            "${GENERATED_REAL_FILE}"
            @ONLY
        )
        file(REMOVE "${GENERATED_LINK_FILE}")
        file(CREATE_LINK
            "${GENERATED_REAL_FILE}"
            "${GENERATED_LINK_FILE}"
            SYMBOLIC
            COPY_ON_ERROR
        )
        include_directories("${GENERATED_LINK_DIR}")
        message(STATUS "Processed configuration file '${config_file_name}' to generate '${GENERATED_REAL_FILE}'")
        message(STATUS "Linked generated header to '${GENERATED_LINK_FILE}'")
    else()
        message(WARNING "Configuration file '${config_file_name}' not found, skipping generation of '${config_header_name}'")
    endif()
endfunction()
