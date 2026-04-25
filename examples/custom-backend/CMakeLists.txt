file(GLOB SRC "*.h" "*.cpp")

add_executable(custom-backend ${SRC})
target_link_libraries(custom-backend elemental)

# link to X11
if (UNIX AND NOT APPLE)
    target_link_libraries(custom-backend X11)
endif()
