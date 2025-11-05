cmake_minimum_required(VERSION 4.0.0)
project(ssf)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_BINARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(data_directory ${CMAKE_BINARY_OUTPUT_DIRECTORY}/data)

file(MAKE_DIRECTORY ${CMAKE_BINARY_OUTPUT_DIRECTORY}/data)

set(root ${CMAKE_CURRENT_SOURCE_DIR})

set(files
  # systems
  "${CMAKE_CURRENT_SOURCE_DIR}/src/gfx/gfx.cpp"

  # core
  "${CMAKE_CURRENT_SOURCE_DIR}/src/core/device.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/core/ssf.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/core/gpu/VK/vk_core.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/core/gpu/VK/vk_helper.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/core/gpu/VK/vk_internal.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/core/gpu/VK/MemoryVK/MemoryVK.cpp"
)

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/src/core/configuration/build_generation.h.in"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/core/configuration/build_generation.h"

)


set(CMAKE_CXX_STANDARD 20)

set(includes)
set(libs)
set(lib_directories
  ${CMAKE_BINARY_DIR}/lib
)

set(include_directories
  ${root}/include
  ${root}/src
  ${root}/external/bcl/src/include
  ${root}/external/bcl/src/include
)


# bcl
set(CMAKE_BUILD_TYPE "Release")
add_subdirectory(${root}/external/bcl/src)

#vulkan 
# TODO: unhardcode please.
# list(APPEND ${libs} vulkan.1.4.328)
list(APPEND lib_directories /Users/brinq/.dev/librarys/VulkanSDK/1.4.328.1/macOS/lib)
list(APPEND include_directories /Users/brinq/.dev/librarys/VulkanSDK/1.4.328.1/macOS/include)

if(APPLE)
  list(APPEND include_directories ${root}/external/glfw/include)
  add_subdirectory(${root}/external/glfw)
  set(macos_frameworks "-framework Metal -framework IOKit -framework Cocoa -framework QuartzCore")
endif()


set(CMAKE_BUILD_TYPE "Debug")

set(CMAKE_CXX_FLAGS
  "-g"
)

add_library(${PROJECT_NAME} ${files})
target_include_directories(${PROJECT_NAME} PRIVATE ${include_directories})

target_link_libraries(${PROJECT_NAME} PRIVATE 
  /Users/brinq/.dev/librarys/VulkanSDK/1.4.328.1/macOS/lib/libvulkan.1.dylib
  ${CMAKE_BINARY_DIR}/lib/libglfw3.a
  ${macos_frameworks}
)

target_link_directories(${PROJECT_NAME} PRIVATE 
  ${lib_directories}
)
