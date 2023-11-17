cmake_minimum_required(VERSION 3.26)

set(CMAKE_SYSTEM_NAME Windows)

project(IgnisEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_library(ignis)

target_include_directories(ignis PRIVATE external/glm)
target_include_directories(ignis PRIVATE external/tinyglTF)
target_include_directories(ignis PRIVATE external/tinyObj)
target_include_directories(ignis PRIVATE external/stb)
target_include_directories(ignis PRIVATE external/imgui)

file(GLOB IMGUI_SOURCE "./external/imgui/*.cpp")

target_sources(ignis PRIVATE ${IMGUI_SOURCE})

find_package(Vulkan REQUIRED OPTIONAL_COMPONENTS shaderc_combined)

target_link_libraries(ignis PRIVATE Vulkan::Vulkan Vulkan::shaderc_combined)

target_compile_definitions(ignis PRIVATE VK_USE_PLATFORM_WIN32_KHR)
target_compile_definitions(ignis PRIVATE TINYOBJLOADER_IMPLEMENTATION)
target_compile_definitions(ignis PRIVATE STB_IMAGE_IMPLEMENTATION)
target_compile_definitions(ignis PRIVATE GLM_FORCE_RADIANS)
target_compile_definitions(ignis PRIVATE GLM_FORCE_DEPTH_ZERO_TO_ONE)

add_subdirectory(src)