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

# Cairo (uses meson upstream, so CPM falls back to system find_package)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
CPMFindPackage(NAME Cairo)

add_definitions(-DSDL_MAIN_HANDLED)

file(GLOB_RECURSE TUI_SRC
	"src/tui/*.h"
	"src/tui/*.cpp"
	"src/tui/*.hpp"
)

add_library(${PROJECT_NAME} ${TUI_SRC})
target_include_directories(${PROJECT_NAME} PUBLIC ${CAIRO_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC SDL3::SDL3 ${CAIRO_LIBRARIES})

add_executable(tui_test "src/main.cpp")
target_link_libraries(tui_test PRIVATE ${PROJECT_NAME})
