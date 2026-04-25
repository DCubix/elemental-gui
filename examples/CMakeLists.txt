set(CPM_DOWNLOAD_LOCATION "${CMAKE_SOURCE_DIR}/cmake/CPM.cmake")
if(NOT EXISTS ${CPM_DOWNLOAD_LOCATION})
    message(ERROR "CPM.cmake not found at ${CPM_DOWNLOAD_LOCATION}. Please run the top-level CMakeLists.txt to download it.")
endif()
include(${CPM_DOWNLOAD_LOCATION})

set(EXAMPLES_ASSETS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/assets")

file(GLOB EXAMPLE_DIRS LIST_DIRECTORIES true "${CMAKE_CURRENT_SOURCE_DIR}/*")

CPMFindPackage(NAME SDL3)

foreach(EXAMPLE_DIR ${EXAMPLE_DIRS})
    # if this the assets directory, skip it
    if(EXAMPLE_DIR STREQUAL EXAMPLES_ASSETS_DIR)
        # copy contents of assets to build directory for examples to use ( only the files )
        file(GLOB ASSET_FILES "${EXAMPLES_ASSETS_DIR}/*")
        foreach(ASSET_FILE ${ASSET_FILES})
            file(COPY ${ASSET_FILE} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
        endforeach()
        continue()
    endif()

    if(NOT IS_DIRECTORY ${EXAMPLE_DIR})
        continue()
    endif()

    get_filename_component(EXAMPLE_NAME ${EXAMPLE_DIR} NAME)

    if(NOT EXISTS "${EXAMPLE_DIR}/main.cpp")
        continue()
    endif()

    add_executable(${EXAMPLE_NAME} "${EXAMPLE_DIR}/main.cpp")
    if (TARGET SDL3::SDL3)
        target_link_libraries(${EXAMPLE_NAME} PRIVATE SDL3::SDL3)
    endif()
    target_link_libraries(${EXAMPLE_NAME} PRIVATE elemental)

    message(STATUS "Added example: ${EXAMPLE_NAME}")
endforeach()
