#pragma once

#include "vk_defines.h"

#include <vector>
#include <array>


//tmp
#include "gfx/prefabs.h"
//---------

struct Device;
class VK;


class VK{
private:

  //static pipeline

  struct Semaphore{
   static constexpr int RenderReady = 0;
   static constexpr int RenderDone = 1;
   static constexpr int RenderSwap = 1;

   static constexpr int Count = 3;
  };

  struct Fence{
    static constexpr int FrameInFlight = 0;
    static constexpr int Count = 1;
  };

  static constexpr uint64_t _stagingBufferSize = 1000000;
  static constexpr int _macosDeviceLocalFlag = 0;
  static constexpr int _macosHostAccessFlag = 1;

  VkInstance instance;
  VkDevice device;
  VkPhysicalDevice gpu;

  VkSurfaceKHR surface;
  VkSwapchainKHR swapchain;
  VkExtent2D swapchainExtent;

  uint8_t numBackbuffers = 2;
  uint32_t curBackBuffer = 0;
  VkImageView swapchainViews[2];
  VkFramebuffer framebuffers[2];
  std::array<VkSemaphore, 2> swapSemaphores;

  VkRenderPass mainRenderpass;
  VkPipeline mainPipeline;

  VkCommandPool graphicsPool;
  VkCommandPool transferPool;

  VkCommandBuffer mainCommandBuffer;
  VkFence mainFence;

  VkQueue graphicQueue;
  VkQueue transferQueue;
  std::vector<QueueFamily> queueFamilies;
  std::pair<VkBuffer, VkDeviceMemory> stagingBuffers[2];

  std::array<VkSemaphore, Semaphore::Count> semaphores;
  std::array<VkFence, Fence::Count> fences;

  ivk::FeatureSet features;

  struct{
    const ssf::prefabs::StandardCube<uint16_t> data;
    VkBuffer vbo;
    VkDeviceMemory vHandle;
    uint64_t vDataSize;
    VkBuffer ibo;
    VkDeviceMemory iHandle;
    uint64_t iDataSize;
  }_TmpCube;

private:
  void SwapBackBuffers();

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
