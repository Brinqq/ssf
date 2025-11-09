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

VkResult CreateImage2D(VkDevice device, VkFormat format, VkExtent3D extent, uint32_t mips,
                       VkImageUsageFlags usage, VkImageLayout layout, VkSampleCountFlagBits MSAA,
                       VkImageTiling VK_IMAGE_TILING_OPTIMAL, uint32_t layers, VkImage* pImage){

  VkImageCreateInfo image{};
  image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  image.pNext = nullptr;
  image.flags = 0;
  image.imageType = VK_IMAGE_TYPE_2D;
  image.format = format;
  image.extent = extent;
  image.usage = usage;
  image.initialLayout = layout;
  image.mipLevels = mips;
  image.samples = VK_SAMPLE_COUNT_1_BIT;
  image.tiling = VK_IMAGE_TILING_OPTIMAL;
  image.arrayLayers = 1;
  image.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  image.queueFamilyIndexCount = 0;
  image.pQueueFamilyIndices = nullptr;
  return vkCreateImage(device, &image, nullptr, pImage);
}

VkResult CreateImageView2D(VkDevice device, VkImage image, VkFormat format, const VkComponentMapping& components
                          ,const VkImageSubresourceRange& range, VkImageView* pImageView){
  VkImageViewCreateInfo view{};
  view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  view.pNext = nullptr;
  view.flags = 0;
  view.image = image;
  view.viewType =  VK_IMAGE_VIEW_TYPE_2D;
  view.format = format;
  view.components = components;
  view.subresourceRange = range;
  return vkCreateImageView(device, &view, nullptr, pImageView);
}


VkResult BeginCommandBuffer(VkCommandBuffer buf, VkCommandBufferUsageFlagBits flag){
  VkCommandBufferBeginInfo buffer{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr,
                                   VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr}; 

  return vkBeginCommandBuffer(buf, &buffer);

}


  VkResult EndCommandBuffer(VkCommandBuffer buf){
    return vkEndCommandBuffer(buf);
  }

  void DestroyImages(VkDevice device, VkImage* pImages, uint32_t count, VkAllocationCallbacks* pAllocator){
    for(int i = 0; i < count; ++i){
      vkDestroyImage(device, pImages[i], pAllocator);
    }
  }

  void DestroyFences(VkDevice device, VkFence* pFences, uint32_t count, VkAllocationCallbacks* pAllocator){
    for(int i = 0; i < count; ++i){
      vkDestroyFence(device, pFences[i], pAllocator);
    }

  }

  void DestroySemaphores(VkDevice device, VkSemaphore* pSemaphores, uint32_t count, VkAllocationCallbacks* pAllocator){
    for(int i = 0; i < count; ++i){
      vkDestroySemaphore(device, pSemaphores[i], pAllocator);
    }

  }

  void DestroyShaders(VkDevice device, VkShaderModule* pShaders, uint32_t count, VkAllocationCallbacks* pAllocator){
    for(int i = 0; i < count; ++i){
      vkDestroyShaderModule(device, pShaders[i],pAllocator);
    }

  }

}
