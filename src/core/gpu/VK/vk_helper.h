#pragma once

#include "vk_defines.h"

#include <bcl/containers/vector.h>

// util functions and thin wrappers over the vulkan api to speed up development.


class GLFWwindow;

namespace vkh{

//platform specific
#if __APPLE__
  VkSurfaceKHR GetPlatformSurface(VkInstance instance, GLFWwindow* handle);
  void GetPlatformExtensions(bcl::small_vector<const char*>& vec);
#endif


#if _WIN32

#endif

#if __LINUX__

#endif


struct PipelineState{
  VkRenderPass renderpass;
  VkPipelineLayout layout;
};

enum VkhImageFlags{
  Multisampling = 0x0,
};

//util functions
void GenerateQueueFamilies(VkPhysicalDevice device, QueueFamily* const pDat, size_t& bytes);
VkPhysicalDevice GetGpu(VkInstance& instance);


VkFormat GetCompatibleSurfaceFormat(VkPhysicalDevice gpu, VkSurfaceKHR surface);
VkColorSpaceKHR GetCompatibleSurfaceColorSpace(VkPhysicalDevice gpu, VkSurfaceKHR surface);
VkExtent2D GetCompatibleSurfaceExtent();

//create info helpers.
//

VkDeviceQueueCreateInfo CreateDeviceQueueCI(uint32_t index, uint32_t count, float p);


//vulkan objects creation helpers.
// 


VkResult CreateImage2D(VkDevice device, VkImage* image, VkFormat format, VkExtent3D extent,
                          VkImageUsageFlagBits usage, VkImageLayout layout);


void DestroyImage(VkDevice device, VkImage* image);

VkPipeline CreatePipeline(VkDevice device, PipelineState& pipeline);
void DestroyPipeline(VkDevice device);

VkRenderPass CreateRenderpass(VkDevice device);
void DestroyRenderpass(VkDevice device);


VkImageView CreateImageView(VkDevice device, VkImage image, VkImageViewType dem, VkFormat format);
void DestroyImageView(VkDevice device, VkImageView view);

VkResult CreateBuffer(VkDevice device, VkBuffer* buf, size_t bytes, const VkBufferUsageFlags usage);
void DestroyBuffer(VkDevice device, VkBuffer buffer);


}//namespace vkh;
