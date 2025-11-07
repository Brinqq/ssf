#include "vk_wrappers.h"

namespace ivk::wrappers{

VkResult CreateShader(VkDevice device, uint32_t* pDat, size_t size, VkAllocationCallbacks* pAllocator, VkShaderModule* pHandle){
  const VkShaderModuleCreateInfo shader{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, nullptr, 0, size, pDat };
  return vkCreateShaderModule(device, &shader,pAllocator, pHandle);
}

VkResult CreateSemaphore(VkDevice device, VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore){
  const VkSemaphoreCreateInfo semaphore{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, nullptr, 0};
  return vkCreateSemaphore(device, &semaphore, pAllocator, pSemaphore);
}

VkResult CreateFence(VkDevice device, VkFenceCreateFlags bit, VkAllocationCallbacks* pAllocator, VkFence* pFence){
  const VkFenceCreateInfo fence{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, nullptr, bit, };
  return vkCreateFence(device, &fence, pAllocator, pFence);
}

VkResult BeginCommandBuffer(VkCommandBuffer buf, VkCommandBufferUsageFlagBits flag){
  VkCommandBufferBeginInfo buffer{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr,
                                    VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr}; 
  return vkBeginCommandBuffer(buf, &buffer);

  }

}
