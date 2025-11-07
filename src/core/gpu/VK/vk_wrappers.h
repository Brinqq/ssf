#pragma once

#include "vulkan/vulkan.h"

namespace ivk::wrappers{
  VkResult CreateShader(VkDevice device, uint32_t* pDat, size_t size, VkAllocationCallbacks* pAllocator, VkShaderModule* pHandle);
  VkResult CreateSemaphore(VkDevice device, VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore);
  VkResult CreateFence(VkDevice device, VkFenceCreateFlags bit, VkAllocationCallbacks* pAllocator, VkFence* pFence);
  VkResult BeginCommandBuffer(VkCommandBuffer buf, VkCommandBufferUsageFlagBits flag = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
}
