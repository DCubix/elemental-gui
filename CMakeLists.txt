cmake_minimum_required(VERSION 3.14)
project(elemental VERSION 0.5.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
  Debug Release RelWithDebInfo MinSizeRel Profile
)

set(CMAKE_C_FLAGS_PROFILE   "${CMAKE_C_FLAGS_RELWITHDEBINFO} -pg"   CACHE STRING "Flags for Profile build")
set(CMAKE_CXX_FLAGS_PROFILE "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -pg" CACHE STRING "Flags for Profile build")
set(CMAKE_EXE_LINKER_FLAGS_PROFILE    "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} -pg"    CACHE STRING "Linker flags for Profile build")
set(CMAKE_SHARED_LINKER_FLAGS_PROFILE "${CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO} -pg" CACHE STRING "Shared linker flags for Profile build")

mark_as_advanced(CMAKE_CXX_FLAGS_PROFILE CMAKE_C_FLAGS_PROFILE CMAKE_EXE_LINKER_FLAGS_PROFILE CMAKE_SHARED_LINKER_FLAGS_PROFILE)

if(CMAKE_CONFIGURATION_TYPES)
  list(APPEND CMAKE_CONFIGURATION_TYPES Profile)
  set(CMAKE_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES}" CACHE STRING "Available configs" FORCE)
endif()

set_property(GLOBAL PROPERTY DEBUG_CONFIGURATIONS Debug Profile)

# Some options
option(ELEMENTAL_BUILD_EXAMPLES "Build example applications" ON)
option(ELEMENTAL_BACKEND_SDL3 "Enable SDL3 backend" ON)

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

# nlohmann/json
CPMAddPackage(
  NAME nlohmann_json
  URL https://github.com/nlohmann/json/releases/download/v3.12.0/include.zip
  DOWNLOAD_ONLY YES
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

# Embed resources as headers before building
set(RESOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resources")
set(GENERATED_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/generated")
execute_process(
	COMMAND ${CMAKE_COMMAND}
		-DRESOURCE_DIR=${RESOURCE_DIR}
		-DOUTPUT_DIR=${GENERATED_DIR}
		-P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/embed_resources.cmake"
)

file(GLOB_RECURSE TUI_SRC
    "src/*.cpp"
    "src/*.hpp"
    "include/elemental/*.h"
)

file(GLOB_RECURSE EXCLUDE_FILES
    "src/backends/*.cpp"
    "include/elemental/backends/*.h"
)

foreach(EXCLUDE_FILE ${EXCLUDE_FILES})
    list(REMOVE_ITEM TUI_SRC ${EXCLUDE_FILE})
endforeach()

if (ELEMENTAL_BACKEND_SDL3)
    file(GLOB_RECURSE SDL3_SRC
        "src/backends/sdl3/*.cpp"
        "include/elemental/backends/sdl3/*.h"
    )
    list(APPEND TUI_SRC ${SDL3_SRC})
endif()

add_library(${PROJECT_NAME} ${TUI_SRC})
target_include_directories(${PROJECT_NAME} PRIVATE ${CAIRO_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${CAIRO_LIBRARIES} nlohmann_json nanosvg)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/elemental
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION   ${PROJECT_VERSION}       # 0.5.1
    SOVERSION ${PROJECT_VERSION_MAJOR} # 0
)

# Backends linking (headers only)
if (ELEMENTAL_BACKEND_SDL3)
  add_definitions(-DSDL_MAIN_HANDLED)
  add_definitions(-DELEMENTAL_BACKEND_SDL3)
  CPMFindPackage(NAME SDL3)
  if (TARGET SDL3::SDL3)
    target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3)
  endif()
endif()

# Re-embed resources before each build
add_custom_target(embed_resources ALL
	COMMAND ${CMAKE_COMMAND}
		-DRESOURCE_DIR=${RESOURCE_DIR}
		-DOUTPUT_DIR=${GENERATED_DIR}
		-P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/embed_resources.cmake"
	COMMENT "Embedding resources into headers"
)
add_dependencies(${PROJECT_NAME} embed_resources)

if (ELEMENTAL_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

install(TARGETS ${PROJECT_NAME}
    EXPORT elementalTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/elemental
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
    PATTERN "backends" EXCLUDE)

if (ELEMENTAL_BACKEND_SDL3)
    install(DIRECTORY include/elemental/backends/sdl3
        DESTINATION include/elemental/backends
        FILES_MATCHING PATTERN "*.h")
endif()

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/elementalConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion)

install(EXPORT elementalTargets
    FILE elementalTargets.cmake
    NAMESPACE elemental::
    DESTINATION lib/cmake/elemental)

set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_GENERATOR "TGZ;ZIP")
include(CPack)