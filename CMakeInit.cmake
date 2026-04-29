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
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_DIRECTORY}" PARENT_SCOPE)

    set(CMAKE_SHARED_LIBRARY_PREFIX "" PARENT_SCOPE)
    set(CMAKE_STATIC_LIBRARY_PREFIX "" PARENT_SCOPE)
    message(STATUS "Stripped library prefixes for shared and static libraries")
endfunction()

function(configure_target)
    cmake_parse_arguments(ARG "" "NAME" "TARGETS;SOURCES;DEPENDS" ${ARGN})

    foreach(TARGET_NAME ${ARG_TARGETS})
        if(NOT "${ARG_NAME}" STREQUAL "")
            set_target_properties(${TARGET_NAME} PROPERTIES
                OUTPUT_NAME "${ARG_NAME}"
            )
        endif()

        if(ARG_SOURCES)
            target_sources(${TARGET_NAME} PRIVATE ${ARG_SOURCES})
        endif()

        if(ARG_DEPENDS)
            target_link_libraries(${TARGET_NAME} PRIVATE ${ARG_DEPENDS})
        endif()
    endforeach()
    if(NOT "${ARG_NAME}" STREQUAL "")
        message(STATUS "Configured target(s) ${ARG_TARGETS} with output name '${ARG_NAME}', sources: ${ARG_SOURCES}, dependencies: ${ARG_DEPENDS}")
    else()
        message(STATUS "Configured target(s) ${ARG_TARGETS} with sources: ${ARG_SOURCES}, dependencies: ${ARG_DEPENDS}")
    endif()
endfunction()


function(define)
    add_compile_definitions(
        METADATA_NAME=\"${METADATA_NAME}\"
        METADATA_VERSION_MAJOR=\"${METADATA_VERSION_MAJOR}\"
        METADATA_VERSION_MINOR=\"${METADATA_VERSION_MINOR}\"
        METADATA_VERSION_PATCH=\"${METADATA_VERSION_PATCH}\"
        METADATA_VERSION=\"${METADATA_VERSION}\"
        METADATA_DESCRIPTION=\"${METADATA_DESCRIPTION}\"
        TARGET_VENDOR=\"${TARGET_VENDOR}\"
        TARGET_SYSTEM=\"${TARGET_SYSTEM}\"
        TARGET_ARCHITECTURE=\"${TARGET_ARCHITECTURE}\"
    )
    message(STATUS "Added compile definition METADATA_NAME=${METADATA_NAME}")
    message(STATUS "Added compile definition METADATA_VERSION_MAJOR=${METADATA_VERSION_MAJOR}")
    message(STATUS "Added compile definition METADATA_VERSION_MINOR=${METADATA_VERSION_MINOR}")
    message(STATUS "Added compile definition METADATA_VERSION_PATCH=${METADATA_VERSION_PATCH}")
    message(STATUS "Added compile definition METADATA_VERSION=${METADATA_VERSION}")
    message(STATUS "Added compile definition METADATA_DESCRIPTION=${METADATA_DESCRIPTION}")
    message(STATUS "Added compile definition TARGET_VENDOR=${TARGET_VENDOR}")
    message(STATUS "Added compile definition TARGET_SYSTEM=${TARGET_SYSTEM}")
    message(STATUS "Added compile definition TARGET_ARCHITECTURE=${TARGET_ARCHITECTURE}")

    # WIN32
    if(TARGET_VENDOR STREQUAL "microsoft" AND (TARGET_SYSTEM MATCHES "^nt" OR TARGET_SYSTEM MATCHES "^win"))
        set(TARGET_PLATFORM_WIN32 1 PARENT_SCOPE)
        add_compile_definitions(TARGET_PLATFORM_WIN32=1)
        message(STATUS "Added compile definition TARGET_PLATFORM_WIN32=1")
    else()
        set(TARGET_PLATFORM_WIN32 0 PARENT_SCOPE)
        add_compile_definitions(TARGET_PLATFORM_WIN32=0)
        message(STATUS "Added compile definition TARGET_PLATFORM_WIN32=0")
    endif()

    if(TARGET_VENDOR STREQUAL "apple" AND TARGET_SYSTEM MATCHES "^darwin")
        set(TARGET_PLATFORM_DARWIN 1 PARENT_SCOPE)
        add_compile_definitions(TARGET_PLATFORM_DARWIN=1)
        message(STATUS "Added compile definition TARGET_PLATFORM_DARWIN=1")
    else()
        set(TARGET_PLATFORM_DARWIN 0 PARENT_SCOPE)
        add_compile_definitions(TARGET_PLATFORM_DARWIN=0)
        message(STATUS "Added compile definition TARGET_PLATFORM_DARWIN=0")
    endif()
endfunction()
