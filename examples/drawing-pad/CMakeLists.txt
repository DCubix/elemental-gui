CPMFindPackage(NAME SDL3)

add_executable(drawing-pad "main.cpp")
if (TARGET SDL3::SDL3)
    target_link_libraries(drawing-pad PRIVATE SDL3::SDL3)
endif()
target_link_libraries(drawing-pad PRIVATE elemental)
