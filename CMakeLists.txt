cmake_minimum_required(VERSION 3.13)
project(FFmAdobe VERSION 1.0.0 LANGUAGES CXX)
set(FFMADOBE_SNAPSHOT_VERSION "26w20a")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============================
# Adobe Premiere Pro SDK Path
# ============================
set(ADOBE_PP_SDK "" CACHE PATH "Location of the Adobe Premiere Pro C++ SDK")
if (NOT IS_DIRECTORY "${ADOBE_PP_SDK}")
    # Try common default location
    if (EXISTS "E:/Code/Premiere Pro 26.0 C++ SDK")
        set(ADOBE_PP_SDK "E:/Code/Premiere Pro 26.0 C++ SDK")
    else()
        message(FATAL_ERROR
            "ADOBE_PP_SDK must be set to the Premiere Pro SDK root directory.\n"
            "  Example: cmake -DADOBE_PP_SDK=\"E:/Code/Premiere Pro 26.0 C++ SDK\" ..")
    endif()
endif()
message(STATUS "Using Adobe PP SDK: ${ADOBE_PP_SDK}")

# ============================
# Architecture
# ============================
if(NOT CMAKE_VS_PLATFORM_NAME)
    set(CMAKE_VS_PLATFORM_NAME "x64")
endif()

# ============================
# Output directories
# ============================
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

# ============================
# MSVC specific settings
# ============================
if (MSVC)
    add_compile_options("$<$<CONFIG:RELEASE>:-MD>")
    add_compile_options("$<$<CONFIG:DEBUG>:-MDd>")
    add_link_options("$<$<CONFIG:RELEASE>:-OPT:REF;-OPT:ICF>")
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

# ============================
# Configure header
# ============================
configure_file("src/configure.h.in" "${PROJECT_BINARY_DIR}/configure/configure.h" @ONLY)
include_directories("${PROJECT_BINARY_DIR}/configure")

# ============================
# Build FFmAdobe plugin
# ============================
add_subdirectory("src/FFmAdobe")
set_target_properties(FFmAdobe PROPERTIES SUFFIX ".prm")

if (MSVC)
    set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY VS_STARTUP_PROJECT FFmAdobe)
endif()
