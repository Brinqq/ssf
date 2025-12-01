#pragma once
#include "vulkan/vulkan.h"

namespace ivk::wrappers{
  VkResult CreateShader(VkDevice device, uint32_t* pDat, size_t size, VkAllocationCallbacks* pAllocator, VkShaderModule* pHandle);
  VkResult CreateSemaphore(VkDevice device, VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore);
  VkResult CreateFence(VkDevice device, VkFenceCreateFlags bit, VkAllocationCallbacks* pAllocator, VkFence* pFence);
  VkResult BeginCommandBuffer(VkCommandBuffer buf, VkCommandBufferUsageFlagBits flag = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
  VkResult EndCommandBuffer(VkCommandBuffer buf);


VkResult CreateImage2D(VkDevice device, VkFormat format, VkExtent3D extent, uint32_t mips,
                       VkImageUsageFlags usage, VkImageLayout layout, VkSampleCountFlagBits MSAA,
                       VkImageTiling VK_IMAGE_TILING_OPTIMAL, uint32_t layers, VkImage* pImage);

VkResult CreateImageView2D(VkDevice device, VkImage image, VkFormat format, const VkComponentMapping& components
                          ,const VkImageSubresourceRange& range, VkImageView* pImageView);

VkResult CreateSampler(VkDevice device, VkSamplerAddressMode mode, bool aniso, uint32_t ansioMax,
                       uint32_t minLod, uint32_t maxLod, const VkAllocationCallbacks* allocator, VkSampler* pSampler);




  void DestroySamplers(VkDevice device, VkSampler* samplers, uint32_t count, const VkAllocationCallbacks* allocator);
  void DestroyImages(VkDevice device, VkImage* pImages, uint32_t count, VkAllocationCallbacks* pAllocator);
  void DestroyImageViews(VkDevice device, VkImageView pViews, uint32_t count, VkAllocationCallbacks* pAllocator);
  void DestroyFences(VkDevice device, VkImage* pFences, uint32_t count, VkAllocationCallbacks* pAllocator);
  void DestroySemaphores(VkDevice device, VkImage* pSemaphores, uint32_t count, VkAllocationCallbacks* pAllocator);
  void DestroyShaders(VkDevice, VkShaderModule* pShaders, uint32_t count, VkAllocationCallbacks* pAllocator);

  // these dont require clean up
  inline VkImageMemoryBarrier CreateImageMemoryBarrier(VkDevice device, VkImage image, uint32_t srcQ, uint32_t dstQ,
         VkAccessFlags srcAccess, VkAccessFlags dstAccess,
         VkImageLayout srcLayout, VkImageLayout dstLayout,
         VkImageSubresourceRange range){
    return VkImageMemoryBarrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, 0, srcAccess, dstAccess, srcLayout, dstLayout, srcQ, dstQ, image, range };
  };
}
