cmake_minimum_required(VERSION 3.26)

set(CMAKE_SYSTEM_NAME Windows)

set(TARGET_NAME Ignis)
project(${TARGET_NAME})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_library(${TARGET_NAME})

target_include_directories(${TARGET_NAME} PRIVATE external/glm)
target_include_directories(${TARGET_NAME} PRIVATE external/tinyglTF)
target_include_directories(${TARGET_NAME} PRIVATE external/tinyObj)
target_include_directories(${TARGET_NAME} PRIVATE external/stb)
target_include_directories(${TARGET_NAME} PRIVATE external/imgui)

file(GLOB IMGUI_SOURCE "./external/imgui/*.cpp")

target_sources(${TARGET_NAME} PRIVATE ${IMGUI_SOURCE})

find_package(Vulkan REQUIRED OPTIONAL_COMPONENTS shaderc_combined)

target_link_libraries(${TARGET_NAME} Vulkan::Vulkan Vulkan::shaderc_combined)

target_compile_definitions(${TARGET_NAME} PRIVATE VK_USE_PLATFORM_WIN32_KHR)
target_compile_definitions(${TARGET_NAME} PRIVATE TINYOBJLOADER_IMPLEMENTATION)
target_compile_definitions(${TARGET_NAME} PRIVATE STB_IMAGE_IMPLEMENTATION)
target_compile_definitions(${TARGET_NAME} PRIVATE GLM_FORCE_RADIANS)
target_compile_definitions(${TARGET_NAME} PRIVATE GLM_FORCE_DEPTH_ZERO_TO_ONE)

add_subdirectory(src)