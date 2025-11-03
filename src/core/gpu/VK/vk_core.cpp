#include "vk_core.h"
#include "vk_debug.h"
#include "vk_helper.h"
#include "vk_defines.h"

#include "core/device.h"

#include <vulkan/vulkan.h>
#include <stdio.h>

#include <bcl/containers/vector.h>


int VK::CreateComputeState(){
  return 0;
}

int VK::CreateGraphicsState(Device& applicationDevice){

  surface = vkh::GetPlatformSurface(instance, static_cast<GLFWwindow*>(applicationDevice.GraphicsWindow));

  //swap chain
  VkSwapchainCreateInfoKHR cSwapchain;
  cSwapchain.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  cSwapchain.flags = 0;
  cSwapchain.minImageCount = 2;
  cSwapchain.imageArrayLayers = 1;
  cSwapchain.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  cSwapchain.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  cSwapchain.queueFamilyIndexCount = 0;
  cSwapchain.pQueueFamilyIndices = nullptr;
  cSwapchain.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  cSwapchain.presentMode = VK_PRESENT_MODE_FIFO_KHR;
  cSwapchain.clipped = VK_TRUE;
  cSwapchain.oldSwapchain = VK_NULL_HANDLE;
  cSwapchain.surface = surface;
  cSwapchain.imageFormat = vkh::GetCompatibleSurfaceFormat(gpu, surface);
  cSwapchain.imageColorSpace = vkh::GetCompatibleSurfaceColorSpace(gpu, surface);
  cSwapchain.imageExtent = vkh::GetCompatibleSurfaceExtent();
  VkSurfaceCapabilitiesKHR k;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &k);
  cSwapchain.preTransform = k.currentTransform;
  vkcall(vkCreateSwapchainKHR(device, &cSwapchain, nullptr, &swapchain))

  uint32_t imageCount;
  VkImage swapchainImages[2];
  vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
  assert(imageCount == 2);
  vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapchainImages);

  for(int i = 0; i < imageCount; ++i){
    swapchainViews[i] = vkh::CreateImageView(device, swapchainImages[i], VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_B8G8R8A8_UNORM);   
  }
  
  mainRenderpass = vkh::CreateRenderpass(device);

   VkFramebufferCreateInfo cFramebuffer1{};
   cFramebuffer1.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
   cFramebuffer1.pNext = nullptr;
   cFramebuffer1.flags = 0;
   cFramebuffer1.pAttachments = &swapchainViews[0];
   cFramebuffer1.attachmentCount = 1;
  
   cFramebuffer1.width = 1920;
   cFramebuffer1.height = 1080;
   cFramebuffer1.layers = 1;
   cFramebuffer1.renderPass = mainRenderpass;
  
   VkFramebufferCreateInfo cFramebuffer2{};
   cFramebuffer2.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
   cFramebuffer2.pNext = nullptr;
   cFramebuffer2.flags = 0;
   cFramebuffer2.pAttachments = &swapchainViews[0];
   cFramebuffer2.attachmentCount = 1;
  
   cFramebuffer2.width = 1920;
   cFramebuffer2.height = 1080;
   cFramebuffer2.layers = 1;

   cFramebuffer1.renderPass = mainRenderpass;
   vkcall(vkCreateFramebuffer(device, &cFramebuffer1, nullptr, &framebuffers[0]))
   vkcall(vkCreateFramebuffer(device, &cFramebuffer2, nullptr, &framebuffers[1]))

  // //shader
  // vkh::PipelineState pipeline{};
  // vkh::CreatePipeline(device, pipeline);
  
  VkCommandPoolCreateInfo cCommandPool{};
  cCommandPool.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  cCommandPool.pNext = nullptr;
  cCommandPool.flags = 0;
  cCommandPool.queueFamilyIndex = 1;
  vkcall(vkCreateCommandPool(device, &cCommandPool, nullptr, &graphicsPool))

  VkCommandBufferAllocateInfo aCommandBuffer{};
   aCommandBuffer.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
   aCommandBuffer.pNext = nullptr;
   aCommandBuffer.commandPool = graphicsPool;
   aCommandBuffer.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
   aCommandBuffer.commandBufferCount = 1;

  vkcall(vkAllocateCommandBuffers(device, &aCommandBuffer, &mainCommandBuffer))

  return 0;
}

int VK::Init(){
  VkApplicationInfo cApp{};
  cApp.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  cApp.pNext = nullptr;
  cApp.pApplicationName = "vkbackend";
  cApp.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  cApp.pEngineName = "vkEngine";
  cApp.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  cApp.apiVersion = VK_API_VERSION_1_0;

  VkInstanceCreateInfo cInstance{};
  cInstance.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  cInstance.pNext = nullptr;
  cInstance.pApplicationInfo = &cApp;
  cInstance.enabledLayerCount = 0;
  cInstance.ppEnabledLayerNames = nullptr;

  bcl::small_vector<const char*> extentions;
  vkh::GetPlatformExtensions(extentions);
  extentions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);

  cInstance.enabledExtensionCount = extentions.size();
  cInstance.ppEnabledExtensionNames = extentions.data();
  cInstance.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
  vkcall(vkCreateInstance(&cInstance, nullptr, &instance))


  //physical device and queue families
  gpu = vkh::GetGpu(instance);

  size_t c =  0;
  vkh::GenerateQueueFamilies(gpu, nullptr, c);
  queueFamilies.resize(c);
  vkh::GenerateQueueFamilies(gpu, queueFamilies.data(), c);

  //logical device

  bcl::small_vector<const char*> deviceExt;
  deviceExt.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

  VkDeviceCreateInfo cDevice{};
  cDevice.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  cDevice.pNext = nullptr;
  cDevice.flags = 0;
  cDevice.enabledExtensionCount = deviceExt.size();
  cDevice.ppEnabledExtensionNames = deviceExt.data();
  cDevice.pEnabledFeatures = nullptr;

  //FIXME(crossplatform support) find a solution for dynamic queue creation.
  VkDeviceQueueCreateInfo* cQueues = (VkDeviceQueueCreateInfo*)alloca(queueFamilies.size() * sizeof(VkDeviceQueueCreateInfo));

  float p = 1.0f;
  for(int i = 0; i < queueFamilies.size(); ++i){
    p -= .1;
    cQueues[i] = vkh::CreateDeviceQueueCI(queueFamilies[i].index, queueFamilies[i].maxQueues, p);
  }

  cDevice.pQueueCreateInfos = cQueues;
  cDevice.queueCreateInfoCount = queueFamilies.size();
  vkcall(vkCreateDevice(gpu, &cDevice, nullptr, &device))

  vkGetDeviceQueue(device, 0, 0, &graphicQueue);
  vkGetDeviceQueue(device, 1, 0, &transferQueue);
  //fixme end

  return 0;
}



void VK::Draw(){

};

void VK::TestTriangle(){

  //vertex buffer
  _TmpCube.vbo = vkh::CreateBuffer(device, _TmpCube.data.VertexBytes, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

  //index buffer
  _TmpCube.ibo = vkh::CreateBuffer(device, _TmpCube.data.IndiceBytes, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);



  //texture 

  //constantbuffer

}

void VK::Destroy(){
  vkDestroyBuffer(device, _TmpCube.ibo, nullptr);
  vkDestroyBuffer(device, _TmpCube.vbo, nullptr);

  vkDestroyDevice(device, nullptr);
  vkDestroyInstance(instance, nullptr);

}
