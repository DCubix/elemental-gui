CPMFindPackage(NAME SDL3)

add_executable(node-editor "main.cpp")
if (TARGET SDL3::SDL3)
    target_link_libraries(node-editor PRIVATE SDL3::SDL3)
endif()
target_link_libraries(node-editor PRIVATE elemental)
