#!/usr/bin/env cmake -P
# embed_resources.cmake
# Converts all files in RESOURCE_DIR into C++ raw-string-literal headers in OUTPUT_DIR.
# Usage: cmake -DRESOURCE_DIR=... -DOUTPUT_DIR=... -P embed_resources.cmake

if(NOT DEFINED RESOURCE_DIR OR NOT DEFINED OUTPUT_DIR)
    message(FATAL_ERROR "RESOURCE_DIR and OUTPUT_DIR must be defined")
endif()

file(MAKE_DIRECTORY "${OUTPUT_DIR}")

file(GLOB resource_files "${RESOURCE_DIR}/*")
foreach(res_file ${resource_files})
    get_filename_component(fname "${res_file}" NAME_WE)
    get_filename_component(ext "${res_file}" EXT)

    set(out_file "${OUTPUT_DIR}/${fname}.h")

    # Only regenerate if source is newer
    if(EXISTS "${out_file}")
        file(TIMESTAMP "${res_file}" src_ts "%Y%m%d%H%M%S" UTC)
        file(TIMESTAMP "${out_file}" dst_ts "%Y%m%d%H%M%S" UTC)
        if(NOT src_ts STRGREATER dst_ts)
            continue()
        endif()
    endif()

    file(READ "${res_file}" content)
    file(WRITE "${out_file}" "R\"(${content})\"")
    message(STATUS "Embedded: ${fname}${ext} -> ${fname}.h")
endforeach()
