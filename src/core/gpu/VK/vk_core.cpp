#include "vk_core.h"
#include "vk_debug.h"
#include "vk_helper.h"
#include "vk_defines.h"
#include "vk_internal.h"
#include "vk_wrappers.h"

#include "MemoryVK/MemoryVK.h"

#include "core/configuration//build_generation.h"
#include "core/device.h"

#include <vulkan/vulkan.h>

#include <stdio.h>

#include <bcl/containers/vector.h>

int VK::CreateComputeState(){
  return 0;
}

void VK::SwapBackBuffers(){
  vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, semaphores[Semaphore::RenderReady], VK_NULL_HANDLE, &curBackBuffer);
}

int VK::CreateGraphicsState(Device& applicationDevice){

  surface = vkh::GetPlatformSurface(instance, static_cast<GLFWwindow*>(applicationDevice.GraphicsWindow));

  VkSurfaceCapabilitiesKHR surfaceInfo;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &surfaceInfo);
  swapchainExtent = surfaceInfo.currentExtent;

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
  cSwapchain.imageExtent = swapchainExtent;
  cSwapchain.preTransform = surfaceInfo.currentTransform;
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
  
   cFramebuffer1.width = swapchainExtent.width;
   cFramebuffer1.height = swapchainExtent.height;
   cFramebuffer1.layers = 1;
   cFramebuffer1.renderPass = mainRenderpass;
  
   VkFramebufferCreateInfo cFramebuffer2{};
   cFramebuffer2.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
   cFramebuffer2.pNext = nullptr;
   cFramebuffer2.flags = 0;
   cFramebuffer2.pAttachments = &swapchainViews[1];
   cFramebuffer2.attachmentCount = 1;
  
   cFramebuffer2.width = swapchainExtent.width;
   cFramebuffer2.height = swapchainExtent.height;
   cFramebuffer2.layers = 1;
   cFramebuffer2.renderPass = mainRenderpass;

   vkcall(vkCreateFramebuffer(device, &cFramebuffer1, nullptr, &framebuffers[0]))
   vkcall(vkCreateFramebuffer(device, &cFramebuffer2, nullptr, &framebuffers[1]))

  vkh::PipelineState pipeline{};

  //compile shaders
  
  ivk::PipelineShaders shaders;

  std::string f(_SSF_GENERATED_SHADER_FOLDER);
  std::string s(_SSF_GENERATED_SHADER_FOLDER);

  f.append("/object.vert.shader");
  s.append("/object.pixel.shader");

  std::pair<void*, size_t> vert = ivk::CompileShaderSource(f.c_str()); 
  std::pair<void*, size_t> pixel = ivk::CompileShaderSource(s.c_str()); 

  VkShaderModule vertexShader;
  VkShaderModule pixelShader;

  vkcall(ivk::wrappers::CreateShader(device, static_cast<uint32_t*>(vert.first), vert.second, nullptr, &vertexShader))
  vkcall(ivk::wrappers::CreateShader(device, static_cast<uint32_t*>(pixel.first), pixel.second, nullptr, &pixelShader))

  vkcall(ivk::CreateGraphicPipeline(device, ivk::PipelineShaders{vertexShader, pixelShader}, mainRenderpass, &mainPipeline))

  VkCommandPoolCreateInfo cCommandPool{};
  cCommandPool.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  cCommandPool.pNext = nullptr;
  cCommandPool.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  cCommandPool.queueFamilyIndex = 0;
  vkcall(vkCreateCommandPool(device, &cCommandPool, nullptr, &graphicsPool))

  VkCommandPoolCreateInfo cCommandPool2{};
  cCommandPool.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  cCommandPool.pNext = nullptr;
  cCommandPool.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  cCommandPool.queueFamilyIndex = 1;
  vkcall(vkCreateCommandPool(device, &cCommandPool, nullptr, &transferPool))

  VkCommandBufferAllocateInfo aCommandBuffer{};
  aCommandBuffer.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  aCommandBuffer.pNext = nullptr;
  aCommandBuffer.commandPool = graphicsPool;
  aCommandBuffer.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  aCommandBuffer.commandBufferCount = 1;

  vkcall(vkAllocateCommandBuffers(device, &aCommandBuffer, &mainCommandBuffer))

  vkcall(vkh::CreateBuffer(device, &stagingBuffers[0].first, _stagingBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT))
  vkcall(vkh::CreateBuffer(device, &stagingBuffers[1].first, _stagingBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT))

  VkMemoryRequirements req{};
  vkGetBufferMemoryRequirements(device, stagingBuffers[0].first, &req);

  vkcall(MemoryVK::Allocate(device, &stagingBuffers[0].second, req.size, _macosHostAccessFlag))
  vkcall(vkBindBufferMemory(device, stagingBuffers[0].first, stagingBuffers[0].second, 0))

  vkGetBufferMemoryRequirements(device, stagingBuffers[1].first, &req);
  vkcall(MemoryVK::Allocate(device, &stagingBuffers[1].second, req.size, _macosHostAccessFlag))
  vkcall(vkBindBufferMemory(device, stagingBuffers[1].first, stagingBuffers[1].second, 0))

  vkcall(vkAllocateCommandBuffers(device, &aCommandBuffer, &mainCommandBuffer))

  VkFenceCreateInfo cFence;
    cFence.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    cFence.pNext = nullptr;
    cFence.flags = 0;
    vkcall(vkCreateFence(device, &cFence, nullptr, &mainFence))

  for(VkSemaphore& s : semaphores){
    vkcall(ivk::wrappers::CreateSemaphore(device, nullptr, &s))
  }

  for(VkSemaphore& s : swapSemaphores){
    vkcall(ivk::wrappers::CreateSemaphore(device, nullptr, &s))
  }

  for(VkFence& s : fences){
    vkcall(ivk::wrappers::CreateFence(device, VK_FENCE_CREATE_SIGNALED_BIT, nullptr, &s))
  }


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

  bcl::small_vector<const char*> instanceLayers;

  instanceLayers.push_back("VK_LAYER_KHRONOS_validation");

  VkInstanceCreateInfo cInstance{};
  cInstance.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  cInstance.pNext = nullptr;
  cInstance.pApplicationInfo = &cApp;

  cInstance.enabledLayerCount = instanceLayers.size();
  cInstance.ppEnabledLayerNames = instanceLayers.data();

  bcl::small_vector<const char*> extentions;
  vkh::GetPlatformExtensions(extentions);
  extentions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
  extentions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);


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
  deviceExt.push_back("VK_KHR_portability_subset");

  VkDeviceCreateInfo cDevice{};
  cDevice.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  cDevice.pNext = nullptr;
  cDevice.flags = 0;
  cDevice.enabledExtensionCount = deviceExt.size();
  cDevice.ppEnabledExtensionNames = deviceExt.data();
  cDevice.pEnabledFeatures = nullptr;

  //FIXME(crossplatform support) find a solution for dynamic queue creation.
  VkDeviceQueueCreateInfo* cQueues = (VkDeviceQueueCreateInfo*)alloca(queueFamilies.size() * sizeof(VkDeviceQueueCreateInfo));

  float* priorities = (float*)alloca(queueFamilies.size() * sizeof(float));
  for(int i = 0; i < queueFamilies.size(); ++i){
    priorities[i] = 1.0f;
    cQueues[i] = vkh::CreateDeviceQueueCI(queueFamilies[i].index, queueFamilies[i].maxQueues, priorities[i]);
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
  vkWaitForFences(device, 1, &fences[Fence::FrameInFlight], VK_TRUE, UINT64_MAX);
  vkResetFences(device, 1, &fences[Fence::FrameInFlight]);
  SwapBackBuffers();

  vkResetCommandBuffer(mainCommandBuffer, 0);

  vkcall(ivk::wrappers::BeginCommandBuffer(mainCommandBuffer))

  VkClearValue col{
    {0.0f, 0.07f, 0.2f, 1.0f}
  };

  VkRenderPassBeginInfo rpass{};
  rpass.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  rpass.pNext = nullptr;
  rpass.renderPass = mainRenderpass;
  rpass.renderArea = VkRect2D{{0,0}, {swapchainExtent.width,swapchainExtent.height}};
  rpass.clearValueCount = 1;
  rpass.pClearValues = &col;
  rpass.framebuffer = framebuffers[curBackBuffer];
  vkCmdBeginRenderPass(mainCommandBuffer, &rpass, VK_SUBPASS_CONTENTS_INLINE);

  vkCmdBindPipeline(mainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mainPipeline);

  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = swapchainExtent.width;
  viewport.height = swapchainExtent.height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  vkCmdSetViewport(mainCommandBuffer, 0, 1, &viewport);

  VkRect2D scissor{};
  scissor.offset = {0, 0};
  scissor.extent = VkExtent2D{swapchainExtent.width, swapchainExtent.height} ;
  vkCmdSetScissor(mainCommandBuffer, 0, 1, &scissor);
  

   VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(mainCommandBuffer, 0, 1, &_TmpCube.vbo, offsets);
  vkCmdBindIndexBuffer(mainCommandBuffer, _TmpCube.ibo, 0, VK_INDEX_TYPE_UINT16);
  vkCmdDrawIndexed(mainCommandBuffer, _TmpCube.data.nIndices, 1 ,0 ,0, 0);

  vkCmdEndRenderPass(mainCommandBuffer);
  vkEndCommandBuffer(mainCommandBuffer);

  VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  VkSubmitInfo submit{};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.pNext = nullptr;
    submit.waitSemaphoreCount = 1;
    submit.pWaitSemaphores = &semaphores[Semaphore::RenderReady];
    submit.pWaitDstStageMask = waitStages;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &mainCommandBuffer;
    submit.signalSemaphoreCount = 1;
    submit.pSignalSemaphores = &swapSemaphores[curBackBuffer];
    vkcall(vkQueueSubmit(graphicQueue, 1, &submit, fences[Fence::FrameInFlight]))

    VkPresentInfoKHR i{};
    i.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    i.pNext = nullptr;
    i.waitSemaphoreCount = 1;
    i.pWaitSemaphores = &swapSemaphores[curBackBuffer];
    i.swapchainCount = 1;
    i.pSwapchains = &swapchain;
    i.pImageIndices = &curBackBuffer;
    i.pResults = nullptr;
    vkQueuePresentKHR(graphicQueue, &i);
};

void VK::TestTriangle(){

  VkMemoryRequirements req{};

  //vertex buffer
  vkcall(vkh::CreateBuffer(device,&_TmpCube.vbo, _TmpCube.data.VertexBytes, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                            VK_BUFFER_USAGE_TRANSFER_DST_BIT))

  vkGetBufferMemoryRequirements(device, _TmpCube.vbo, &req);
  _TmpCube.vDataSize = req.size;
  vkcall(MemoryVK::Allocate(device, &_TmpCube.vHandle, req.size, _macosDeviceLocalFlag))
  vkcall(vkBindBufferMemory(device, _TmpCube.vbo, _TmpCube.vHandle, 0))

  //index buffer
  vkcall(vkh::CreateBuffer(device, &_TmpCube.ibo, _TmpCube.data.IndiceBytes, VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
                          VK_BUFFER_USAGE_TRANSFER_DST_BIT))

  vkGetBufferMemoryRequirements(device, _TmpCube.ibo, &req);
  _TmpCube.iDataSize = req.size;

  vkcall(MemoryVK::Allocate(device, &_TmpCube.iHandle, req.size, _macosDeviceLocalFlag))
  vkcall(vkBindBufferMemory(device, _TmpCube.ibo, _TmpCube.iHandle, 0))


  void* dat;
  void* data;
  vkcall(vkMapMemory(device, stagingBuffers[0].second, 0, VK_WHOLE_SIZE, 0, &dat))
  memcpy(dat, _TmpCube.data.vertices, _TmpCube.data.VertexBytes);
  vkUnmapMemory(device, stagingBuffers[0].second);

  vkcall(vkMapMemory(device, stagingBuffers[1].second, 0, VK_WHOLE_SIZE, 0, &data))
  memcpy(data, _TmpCube.data.indices, _TmpCube.data.IndiceBytes);
  vkUnmapMemory(device, stagingBuffers[1].second);

  ivk::CopyBufferOp cpy[2]{
    {stagingBuffers[0].first, _TmpCube.vbo, 0, 0, _TmpCube.data.VertexBytes},
    {stagingBuffers[1].first, _TmpCube.ibo, 0, 0, _TmpCube.data.IndiceBytes},
  };

  ivk::CopyBuffers(device, mainCommandBuffer, cpy, 2);

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.pNext = nullptr;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &mainCommandBuffer;

  vkcall(vkQueueSubmit(graphicQueue, 1, &submitInfo, mainFence))
  vkcall(vkQueueWaitIdle(graphicQueue))
  vkWaitForFences(device, 1, &mainFence, VK_TRUE, UINT64_MAX);

  //texture 
  //create image
  
  VkImage tex;
  VkImageCreateInfo cImage{};
// cImage.sType = VK_STRUCTURE
//   vkCreateImage(device, nullptr, &tex);


  //constantbuffer
}

void VK::Destroy(){
  vkDestroyBuffer(device, _TmpCube.ibo, nullptr);
  vkDestroyBuffer(device, _TmpCube.vbo, nullptr);

  vkDestroyDevice(device, nullptr);
  vkDestroyInstance(instance, nullptr);

}
