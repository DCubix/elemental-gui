CPMFindPackage(NAME SDL3)

file(GLOB_RECURSE SRC "*.h" "*.cpp")

add_executable(pixel-paint ${SRC})
if (TARGET SDL3::SDL3)
    target_link_libraries(pixel-paint PRIVATE SDL3::SDL3)
endif()
target_link_libraries(pixel-paint PRIVATE elemental)
