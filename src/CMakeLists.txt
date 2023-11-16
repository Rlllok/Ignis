target_include_directories(ignis PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(app)
add_subdirectory(input)
add_subdirectory(vk)
add_subdirectory(utils)
add_subdirectory(ui)