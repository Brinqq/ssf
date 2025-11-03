#pragma once

#include "vk_defines.h"
#include <vector>

//tmp
#include "gfx/prefabs.h"
//---------

struct Device;

class VK{
    VkInstance instance;
    VkDevice device;
    VkPhysicalDevice gpu;

    VkSurfaceKHR surface;
    VkSwapchainKHR swapchain;
    VkImageView swapchainViews[2];
    VkFramebuffer framebuffers[2];
    VkRenderPass mainRenderpass;

    VkCommandPool graphicsPool;
    VkCommandBuffer mainCommandBuffer;
    VkQueue graphicQueue;
    VkQueue transferQueue;

    std::vector<QueueFamily> queueFamilies;


    struct{
      const ssf::prefabs::StandardCube<uint16_t> data;
      VkBuffer vbo;
      VkBuffer ibo;

    }_TmpCube;

public:
  int Init();
  void Destroy();

  int CreateComputeState();
  int DestroyComputeState();

  int CreateGraphicsState(Device& device);
  int DestroyGraphicsState();

  void Draw();
  void TestTriangle();

};//class VK
