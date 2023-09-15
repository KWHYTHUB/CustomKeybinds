cmake_minimum_required(VERSION 3.3.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

project(CustomKeybinds VERSION 1.1.2)

file(GLOB SOURCES
	src/*.hpp
	src/*.cpp
)

file(GLOB OBJC_SOURCES
    src/*.mm
)
set_source_files_properties(${OBJC_SOURCES} PROPERTIES SKIP_PRECOMPILE_HEADERS ON)

add_library(${PROJECT_NAME} SHARED ${SOURCES} ${OBJC_SOURCES})

if (PROJECT_IS_TOP_LEVEL)
    target_compile_definitions(${PROJECT_NAME} PRIVATE HJFOD_CUSTOM_KEYBINDS_EXPORTING)
endif()

if (NOT DEFINED ENV{SAPPHIRE_SDK})
    message(FATAL_ERROR "Unable to find Sapphire SDK! Please define SAPPHIRE_SDK environment variable to point to Sapphire")
else()
    message(STATUS "Found Sapphire: $ENV{SAPPHIRE_SDK}")
endif()

add_subdirectory($ENV{SAPPHIRE_SDK} $ENV{SAPPHIRE_SDK}/build)

target_link_libraries(${PROJECT_NAME} sapphire)
create_sapphire_file(${PROJECT_NAME})
