cmake_minimum_required(VERSION 3.22)

set(ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

# ---- Options ----

option(COPY_OUTPUT "Copy Output into Skyrim Directory" OFF)
option(BUILD_AE "Build with Skyrim AE Configuration" OFF)

# ---- Project ----

project(
    YameteKudasai
    VERSION 1.2.0
    LANGUAGES CXX
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/CMake/Plugin.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/Plugin.h
    @ONLY
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/CMake/version.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/version.rc
    @ONLY
)

# ---- Globals ----

set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMake")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- Include guards ----

if("${PROJECT_SOURCE_DIR}" STREQUAL "${PROJECT_BINARY_DIR}")
    message(FATAL_ERROR "In-source builds are not allowed.")
endif()

# ---- Cache build vars ----
macro(set_from_environment VARIABLE)
	if (NOT DEFINED ${VARIABLE} AND DEFINED ENV{${VARIABLE}})
		set(${VARIABLE} $ENV{${VARIABLE}})
	endif ()
endmacro()

set_from_environment(Skyrim64Path)
set_from_environment(CommonLibSSEPath)

# ---- Dependencies ----

add_subdirectory(${CommonLibSSEPath} CommonLibSSE)

find_package(spdlog REQUIRED CONFIG)
find_package(yaml-cpp REQUIRED)
# find_package(magic_enum REQUIRED)

# ---- Add source files ----

include(CMake/source-list.cmake)

source_group(
    TREE "${CMAKE_CURRENT_SOURCE_DIR}"
    FILES ${CXX_FILES}
)

source_group(
    TREE "${CMAKE_CURRENT_BINARY_DIR}"
    FILES "${CMAKE_CURRENT_BINARY_DIR}/include/Plugin.h"
)

# ---- Create DLL ----

if (BUILD_AE)
	add_compile_definitions(
		SKYRIM_SUPPORT_AE
	)
endif()

add_library(
    "${PROJECT_NAME}"
    SHARED
        ${CXX_FILES}
        "${CMAKE_CURRENT_BINARY_DIR}/include/Plugin.h"
        "${CMAKE_CURRENT_BINARY_DIR}/version.rc"
        ".clang-format"
        ".editorconfig"
)

target_compile_features(
    "${PROJECT_NAME}"
    PRIVATE
        cxx_std_20
)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    target_compile_options(
        "${PROJECT_NAME}"
        PRIVATE
            "/sdl"   # Enable Additional Security Checks
            "/utf-8" # Set Source and Executable character sets to UTF-8
            "/Zi"    # Debug Information Format

            "/permissive-"     # Standards conformance
            "/Zc:preprocessor" # Enable preprocessor conformance mode

            "/wd4201" # nonstandard extension used : nameless struct/union

            "$<$<CONFIG:DEBUG>:>"
            "$<$<CONFIG:RELEASE>:/Zc:inline;/JMC-;/Ob3>"
    )

    target_link_options(
        "${PROJECT_NAME}"
        PRIVATE
            "$<$<CONFIG:DEBUG>:/INCREMENTAL;/OPT:NOREF;/OPT:NOICF>"
            "$<$<CONFIG:RELEASE>:/INCREMENTAL:NO;/OPT:REF;/OPT:ICF;/DEBUG:FULL>"
    )
endif()

target_include_directories(
    "${PROJECT_NAME}"
    PRIVATE
        "${CMAKE_CURRENT_BINARY_DIR}/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_libraries(
    "${PROJECT_NAME}"
    PUBLIC
        CommonLibSSE::CommonLibSSE
        spdlog::spdlog
        yaml-cpp
)

target_precompile_headers(
    "${PROJECT_NAME}"
    PRIVATE
        "src/PCH.h"
)

# ---- File copying ----

if("${COPY_OUTPUT}")
    if(DEFINED Skyrim64Path)
        add_custom_command(
            TARGET "${PROJECT_NAME}"
            POST_BUILD
                COMMAND "${CMAKE_COMMAND}" -E make_directory "${Skyrim64Path}/Data/SKSE/Plugins/"
                COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:${PROJECT_NAME}>" "${Skyrim64Path}/Data/SKSE/Plugins/"
            VERBATIM
        )
    else()
        message(WARNING "Variable Skyrim64Path is not defined. Skipping post-build copy command.")
    endif()
endif()
