cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "iOS" OR IOS)
	set(CMAKE_OSX_ARCHITECTURES "arm64")
else()
	set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

project(MenuLoopRandomizer VERSION 1.0.0)

file(GLOB SOURCES
    src/*.cpp
    src/*.hpp
    src/ui/*.cpp
    src/ui/*.hpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK! Please define GEODE_SDK environment variable to point to Geode")
else()
    message(STATUS "Found Geode: $ENV{GEODE_SDK}")
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

CPMAddPackage("gh:mackron/miniaudio#f40cf03")
target_link_libraries(${PROJECT_NAME} miniaudio)

# https://miniaud.io/docs/manual/index.html
# The macOS build should compile cleanly without the need to download any dependencies
# nor link to any libraries or frameworks.
# The iOS build needs to be compiled as Objective-C and will need to link the relevant frameworks
# but should compile cleanly out of the box with Xcode.
# Compiling through the command line requires linking to -lpthread and -lm.

if ("${CMAKE_SYSTEM_NAME}" STREQUAL "iOS" OR IOS)
    set_source_files_properties(${miniaudio_SOURCE_DIR}/miniaudio.c PROPERTIES LANGUAGE OBJC)
    target_link_libraries(miniaudio PUBLIC
        "-framework AudioToolbox"
        "-framework CoreAudio"
        "-framework CoreFoundation"
        pthread
        m
    )
endif()

setup_geode_mod(${PROJECT_NAME})
