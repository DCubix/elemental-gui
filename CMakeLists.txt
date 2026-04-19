cmake_minimum_required(VERSION 3.14)
project(elemental VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Auto-download CPM.cmake into cmake/
set(CPM_DOWNLOAD_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/cmake/CPM.cmake")
if(NOT EXISTS ${CPM_DOWNLOAD_LOCATION})
    message(STATUS "Downloading CPM.cmake...")
    file(DOWNLOAD
        https://github.com/cpm-cmake/CPM.cmake/releases/latest/download/get_cpm.cmake
        ${CPM_DOWNLOAD_LOCATION}
    )
endif()
include(${CPM_DOWNLOAD_LOCATION})

# SDL3
CPMAddPackage(
    NAME SDL3
    GITHUB_REPOSITORY libsdl-org/SDL
    GIT_TAG release-3.4.4
    EXCLUDE_FROM_ALL YES
    SYSTEM YES
    OPTIONS
        "SDL_SHARED OFF"
        "SDL_STATIC ON"
        "SDL_TEST_LIBRARY OFF"
		"SDL_X11_XSCRNSAVER OFF"
)

# nlohmann/json
CPMAddPackage(
  NAME nlohmann_json
  VERSION 3.7.3
  # the git repo is incredibly large, so we download the archived include directory
  URL https://github.com/nlohmann/json/releases/download/v3.7.3/include.zip
  URL_HASH SHA256=87b5884741427220d3a33df1363ae0e8b898099fbc59f1c451113f6732891014
)

if (nlohmann_json_ADDED)
  add_library(nlohmann_json INTERFACE IMPORTED)
  target_include_directories(nlohmann_json INTERFACE ${nlohmann_json_SOURCE_DIR}/include)
endif()

# nanosvg
CPMAddPackage(
  NAME nanosvg
  GITHUB_REPOSITORY memononen/nanosvg
  GIT_TAG master
  DOWNLOAD_ONLY YES
)

if (nanosvg_ADDED)
  add_library(nanosvg INTERFACE IMPORTED)
  target_include_directories(nanosvg INTERFACE ${nanosvg_SOURCE_DIR}/src)
endif()

# Cairo (uses meson upstream, so CPM falls back to system find_package)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
CPMFindPackage(NAME Cairo)

find_package(OpenGL REQUIRED)

add_definitions(-DSDL_MAIN_HANDLED)

# Embed resources as headers before building
set(RESOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resources")
set(GENERATED_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/tui/generated")
execute_process(
	COMMAND ${CMAKE_COMMAND}
		-DRESOURCE_DIR=${RESOURCE_DIR}
		-DOUTPUT_DIR=${GENERATED_DIR}
		-P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/embed_resources.cmake"
)

file(GLOB_RECURSE TUI_SRC
	"src/tui/*.h"
	"src/tui/*.cpp"
	"src/tui/*.hpp"
)

add_library(${PROJECT_NAME} ${TUI_SRC})
target_include_directories(${PROJECT_NAME} PUBLIC ${CAIRO_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC SDL3::SDL3 ${CAIRO_LIBRARIES} nlohmann_json nanosvg OpenGL::GL)

# Re-embed resources before each build
add_custom_target(embed_resources ALL
	COMMAND ${CMAKE_COMMAND}
		-DRESOURCE_DIR=${RESOURCE_DIR}
		-DOUTPUT_DIR=${GENERATED_DIR}
		-P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/embed_resources.cmake"
	COMMENT "Embedding resources into headers"
)
add_dependencies(${PROJECT_NAME} embed_resources)

add_executable(tui_test "src/main.cpp")
target_link_libraries(tui_test PRIVATE ${PROJECT_NAME})
