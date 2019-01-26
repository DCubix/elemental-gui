cmake_minimum_required(VERSION 3.10)
project(elemental VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(SDL2 REQUIRED SDL2)
find_package(Cairo REQUIRED Cairo)

add_definitions(-DSDL_MAIN_HANDLED)

file(GLOB_RECURSE TUI_SRC
	"src/tui/*.h"
	"src/tui/*.cpp"
	"src/tui/*.hpp"
)

add_library(${PROJECT_NAME} ${TUI_SRC})
target_link_libraries(${PROJECT_NAME} PRIVATE SDL2 Cairo)

add_executable(tui_test "src/main.cpp")
target_link_libraries(tui_test PRIVATE ${PROJECT_NAME})
