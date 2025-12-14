#pragma once
#include "types.h"
#include <stdint.h>

#include <vulkan/vulkan.h>

#include <vector>

constexpr int kMaxNameLength = 10;
typedef void* ShaderHandle;

struct ShaderRequirements{

};

struct ShaderResources{
  std::vector<VkPipelineShaderStageCreateInfo> shaders;
  std::vector<VkVertexInputBindingDescription> inputs;
  std::vector<VkVertexInputAttributeDescription> attributes;
  std::vector<VkPushConstantRange> pushRanges;
};

struct ShaderContainer{
  ShaderResources resources;
  ShaderRequirements requirements;
  ShaderHandle handle;
};

ShaderContainer  BuildShaderFromMetaFile(VkDevice device, VkAllocationCallbacks* allocator, const char* pMetaFile);
void ShaderFreeContainer(VkDevice device, VkAllocationCallbacks*, ShaderContainer* container);

