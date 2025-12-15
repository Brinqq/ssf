cmake_minimum_required(VERSION 4.0.0)
project(juye)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_BINARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

file(MAKE_DIRECTORY ${CMAKE_BINARY_OUTPUT_DIRECTORY}/data)

set(root ${CMAKE_CURRENT_SOURCE_DIR})

set(runtime_data ${CMAKE_BINARY_OUTPUT_DIRECTORY}/data/)
set(atexture_directory ${root}/data/textures/)

set(compile_samples TRUE)

set(files
  # systems
  "${CMAKE_CURRENT_SOURCE_DIR}/src/gk/gk.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/gk/prefabs.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/gk/actions.cpp"

  # core
  "${CMAKE_CURRENT_SOURCE_DIR}/src/core/juye.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/core/drivers/device.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/core/drivers/VK/vk_core.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/core/drivers/VK/vk_helper.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/core/drivers/VK/vkinternal.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/core/drivers/VK/vk_wrappers.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/core/drivers/VK/MemoryVK/MemoryVK.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/core/drivers/VK/vkshader.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/core/fsystem/image.cpp"
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
  ${root}/external
  ${root}/external/bcl/src/include
  ${root}/external/glm
  ${root}/external/yaml-cpp/include
)


# bcl
set(CMAKE_BUILD_TYPE "Release")
add_subdirectory(${root}/external/bcl/src)
add_subdirectory(${root}/external/yaml-cpp)

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

target_link_libraries(${PROJECT_NAME} PUBLIC 
  /Users/brinq/.dev/librarys/VulkanSDK/1.4.328.1/macOS/lib/libvulkan.1.dylib
  ${CMAKE_BINARY_DIR}/lib/libglfw3.a
  yaml-cpp
  ${macos_frameworks}
)

target_link_directories(${PROJECT_NAME} PRIVATE 
  ${lib_directories}
)


if(compile_samples)
  set (sample_include ${root}/include)
  set (sample_out ${CMAKE_BINARY_OUTPUT_DIRECTORY}/samples)
  set (sample_lib ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

  add_executable(HelloTriangle ${root}/samples/hellotriangle.cpp)
  target_link_directories(HelloTriangle PRIVATE ${sample_lib})
  target_include_directories(HelloTriangle PRIVATE ${sample_include})
  target_link_directories(HelloTriangle PRIVATE ${sample_link})
  set_target_properties(HelloTriangle PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${sample_out})
endif()
