CPMFindPackage(NAME SDL3)

add_executable(elements "main.cpp")
if (TARGET SDL3::SDL3)
    target_link_libraries(elements PRIVATE SDL3::SDL3)
endif()
target_link_libraries(elements PRIVATE elemental)
