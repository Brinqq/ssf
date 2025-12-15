#include "vk_core.h"
#include "vk_debug.h"
#include "vk_helper.h"
#include "vkdefines.h"
#include "vkinternal.h"
#include "vk_wrappers.h"
#include "MemoryVK/MemoryVK.h"
#include "vkshader.h"

#include "core/configuration//build_generation.h"
#include "core/device.h"
#include "core/fsystem/file.h"
#include "vulkan/vulkan_core.h"
#include "core/debug.h"

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdio.h>

#include <bcl/containers/vector.h>
#include <bcl/containers/span.h>
#include <bcl/containers/bucket.h>


//opaque structure implementations

int VK::CreateComputeState(){
  return 0;
}

void VK::SwapBackBuffers(){
  vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, semaphores[Semaphore::RenderReady], VK_NULL_HANDLE, &curBackBuffer);
}


int VK::CreateRenderPass(const bk::span<AttachmentDescription>& attachments, uint32_t numSubpasses, const RenderPassCreateFlags flags, VkRenderPass* pRenderpass){
  const int kSubpassSpillThreshold = 4;
  const int kAttachmentSpillThreshold = 10;

  if(numSubpasses > kMaxSubpasses){
    ssf_runtime_error();
  }

  struct Subpass{
    std::vector<VkAttachmentReference> read;
    std::vector<VkAttachmentReference> write;
    std::vector<uint32_t> preserve;
  };

  Subpass subpasses[kMaxSubpasses];
  VkSubpassDescription dSubpasses[kMaxSubpasses];

  std::vector<VkAttachmentDescription> attachs;
  std::vector<VkAttachmentReference> refs;

  VkAttachmentReference* depthRef = nullptr;

  int i = 0;
  for(AttachmentDescription& e : attachments){
    VkAttachmentDescription atDescription{};
    VkAttachmentReference atRef{};

    atRef.attachment = i;
    atRef.layout = e.gpuRefLayout;

    atDescription.flags = 0;
    atDescription.format = e.format;
    atDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    atDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    atDescription.finalLayout = e.finalLayout;
    
    //TODO: For now we can do this because we dont have any attachments that
    // need to presist across renderpasses, note image layout undefined destorys image data.
    // make these options configurable.
    atDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    atDescription.storeOp =  VK_ATTACHMENT_STORE_OP_STORE;
    atDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    atDescription.samples = VK_SAMPLE_COUNT_1_BIT;

    
    //append(read, write, preserve)
    for(int k = 0; k < numSubpasses; ++k){

      switch(e.usage[k]){

        case RenderAttachmentUsageWrite:
          subpasses[k].write.push_back(atRef);
          break;

        case RenderAttachmentUsageRead:
          subpasses[k].read.push_back(atRef);
          break;

        case RenderAttachmentUsagePreserve:
          subpasses[k].preserve.push_back(i);
          break;

        default:
          ssf_runtime_error();
    }
  }

    attachs.push_back(atDescription);
    refs.push_back(atRef);
    i++;
  }

  if(flags & RenderPassCreateFlags::RenderPassDepthBit){
    VkAttachmentDescription atDescription{};
    VkAttachmentReference atRef{};

    atRef.attachment = attachs.size();
    atRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    atDescription.flags = 0;
    atDescription.format = VK_FORMAT_D32_SFLOAT;
    atDescription.initialLayout =  VK_IMAGE_LAYOUT_UNDEFINED;
    atDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    atDescription.samples = VK_SAMPLE_COUNT_1_BIT;
    atDescription.loadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
    atDescription.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    atDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    atDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  
    refs.push_back(atRef);
    attachs.push_back(atDescription);
    depthRef = &refs.back();
  }

  for(int x = 0; x < numSubpasses; ++x){
    dSubpasses[x].flags = 0;
    dSubpasses[x].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    dSubpasses[x].inputAttachmentCount = subpasses[x].read.size();
    dSubpasses[x].pInputAttachments = subpasses[x].read.data();
    dSubpasses[x].colorAttachmentCount = subpasses[x].write.size();
    dSubpasses[x].pColorAttachments = subpasses[x].write.data();
    dSubpasses[x].preserveAttachmentCount = subpasses[x].preserve.size();
    dSubpasses[x].pPreserveAttachments = subpasses[x].preserve.data();
    dSubpasses[x].pDepthStencilAttachment = depthRef;
    dSubpasses[x].pResolveAttachments = nullptr;
  }
  
  VkSubpassDependency dependency{};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
  dependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

  VkRenderPassCreateInfo cRenderpass{};
  cRenderpass.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  cRenderpass.pNext = nullptr;
  cRenderpass.flags = 0;
  cRenderpass.attachmentCount = attachs.size();
  cRenderpass.pAttachments = attachs.data();
  cRenderpass.subpassCount = numSubpasses;
  cRenderpass.pSubpasses = dSubpasses;
  cRenderpass.dependencyCount = 1;
  cRenderpass.pDependencies = &dependency;

  vkcall(vkCreateRenderPass(device, &cRenderpass, nullptr, pRenderpass))
  return 0;
}

int VK::CreateGraphicsState(Device& applicationDevice){
  
  surface = vkh::GetPlatformSurface(instance, static_cast<GLFWwindow*>(applicationDevice.GraphicsWindow));

  VkSurfaceCapabilitiesKHR surfaceInfo;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &surfaceInfo);
  swapchainExtent = surfaceInfo.currentExtent;

  //swap chain
  VkSwapchainCreateInfoKHR cSwapchain; cSwapchain.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
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
    swapchainViews[i] = vkh::CreateImageView(device, swapchainImages[i], VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);   
  }

  vkcall(ivk::wrappers::CreateImage(device, VK_FORMAT_D32_SFLOAT, 
                                      VkExtent3D{swapchainExtent.width, swapchainExtent.height, 1}, 
                                      VK_IMAGE_TYPE_2D, 1, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                      VK_IMAGE_LAYOUT_UNDEFINED, VK_SAMPLE_COUNT_1_BIT,
                                      VK_IMAGE_TILING_OPTIMAL, 1, 0,&depthBuffer.image
                                      ))

  VkMemoryRequirements depthRequirements;
  vkGetImageMemoryRequirements(device, depthBuffer.image, &depthRequirements);
  vkcall(MemoryVK::Allocate(device, &depthBuffer.memory, depthRequirements.size, _macosDeviceLocalFlag))
  vkcall(vkBindImageMemory(device, depthBuffer.image, depthBuffer.memory, 0))
  depthBuffer.view = vkh::CreateImageView(device, depthBuffer.image, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_D32_SFLOAT, VK_IMAGE_ASPECT_DEPTH_BIT);

  AttachmentDescription f{
    VK_FORMAT_B8G8R8A8_UNORM,
    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    {RenderAttachmentUsageWrite},
  };

  CreateRenderPass(bk::span(&f, 1), 1, RenderPassDepthBit, &mainRenderpass);

  VkImageView frontrp[2]{swapchainViews[0], depthBuffer.view};
  VkImageView backrp[2]{swapchainViews[1], depthBuffer.view};

   VkFramebufferCreateInfo cFramebuffer1{};
   cFramebuffer1.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
   cFramebuffer1.pNext = nullptr;
   cFramebuffer1.flags = 0;
   cFramebuffer1.pAttachments = frontrp;
   cFramebuffer1.attachmentCount = 2;
   cFramebuffer1.width = swapchainExtent.width;
   cFramebuffer1.height = swapchainExtent.height;
   cFramebuffer1.layers = 1;
   cFramebuffer1.renderPass = mainRenderpass;
  
   VkFramebufferCreateInfo cFramebuffer2{};
   cFramebuffer2.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
   cFramebuffer2.pNext = nullptr;
   cFramebuffer2.flags = 0;
   cFramebuffer2.pAttachments = backrp;
   cFramebuffer2.attachmentCount = 2;
   cFramebuffer2.width = swapchainExtent.width;
   cFramebuffer2.height = swapchainExtent.height;
   cFramebuffer2.layers = 1;
   cFramebuffer2.renderPass = mainRenderpass;

   vkcall(vkCreateFramebuffer(device, &cFramebuffer1, nullptr, &framebuffers[0]))
   vkcall(vkCreateFramebuffer(device, &cFramebuffer2, nullptr, &framebuffers[1]))
  
  textureDP.resize(3);
  tCreateDescriptorPools(DescriptorPoolTexture, 3, textureDP.data());

  CreateFixedSamplers(false);
  CreateFixedDescriptors();


  // ------------------------------------------------------------------------------------
  //  for now we hardcode these because i just dont know how
  //  handle these for now
  // ------------------------------------------------------------------------------------
  VkDescriptorSetLayoutBinding geoBindings{};
  geoBindings.binding = 0;
  geoBindings.descriptorCount = 1;
  geoBindings.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  geoBindings.pImmutableSamplers = nullptr;
  geoBindings.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
  VkDescriptorSetLayoutCreateInfo geoLayoutInfo{};
  geoLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  geoLayoutInfo.bindingCount = 1;
  geoLayoutInfo.pBindings = &geoBindings;

  VkDescriptorSetLayoutBinding skyboxBindings{};
  skyboxBindings.binding = 0;
  skyboxBindings.descriptorCount = 1;
  skyboxBindings.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  skyboxBindings.pImmutableSamplers = nullptr;
  skyboxBindings.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
  VkDescriptorSetLayoutCreateInfo nskyboxLayoutInfo{};
  nskyboxLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  nskyboxLayoutInfo.bindingCount = 1;
  nskyboxLayoutInfo.pBindings = &skyboxBindings;

  const char* kGeometryPipelineMetaPath = "/Users/brinq/.dev/projects/solar-sim/ssf/data/shaders/builtin_geometrypass.meta.yaml";
  const char* kSkyboxPipelineMetaPath = "/Users/brinq/.dev/projects/solar-sim/ssf/data/shaders/builtin_skybox.meta.yaml";

  ShaderContainer geometryPassShader =  BuildShaderFromMetaFile(device, nullptr, kGeometryPipelineMetaPath);
  ShaderContainer skyBoxShader =  BuildShaderFromMetaFile(device, nullptr, kSkyboxPipelineMetaPath);

  geoPassDescriptorLayout = &descriptorSetLayoutLut.construct();
  skyboxDescriptorLayout = &descriptorSetLayoutLut.construct();

  vkcall(vkCreateDescriptorSetLayout(device, &geoLayoutInfo, nullptr, geoPassDescriptorLayout))
  vkcall(vkCreateDescriptorSetLayout(device, &geoLayoutInfo, nullptr, skyboxDescriptorLayout))

  VkDescriptorSetAllocateInfo skyboxAlloc{};
  skyboxAlloc.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  skyboxAlloc.pNext = nullptr;
  skyboxAlloc.pSetLayouts = skyboxDescriptorLayout;
  skyboxAlloc.descriptorPool = textureDP.at(0).pool;
  skyboxAlloc.descriptorSetCount = 1;

  vkcall(vkAllocateDescriptorSets(device, &skyboxAlloc, &skyboxDescriptorSet))

  vkcall(ivk::CreatePipelineLayoutFromContainer(device, geometryPassShader, bk::span(geoPassDescriptorLayout, 1), &geoPassPipelineLayout))
  vkcall(ivk::CreatePipelineLayoutFromContainer(device, skyBoxShader, bk::span(skyboxDescriptorLayout, 1), &skyboxPipelineLayout))

  vkcall(ivk::CreateGraphicPipeline(device, geometryPassShader, geoPassPipelineLayout, mainRenderpass, &mainPipeline))
  vkcall(ivk::CreateGraphicPipeline(device, skyBoxShader, skyboxPipelineLayout, mainRenderpass, &skyboxPipeline))

  ShaderFreeContainer(device, nullptr, &geometryPassShader);
  ShaderFreeContainer(device, nullptr, &skyBoxShader);
  // ------------------------------------------------------------------------------------
  // ------------------------------------------------------------------------------------



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

  // vkcall(vkh::CreateBuffer(device, &stagingBuffers[1].first, _stagingBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT))
  // vkcall(vkh::CreateBuffer(device, &stagingBuffers[2].first, _stagingBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT))
  // vkcall(vkh::CreateBuffer(device, &stagingBuffers[3].first, _stagingBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT))
  // vkcall(vkh::CreateBuffer(device, &stagingBuffers[4].first, _stagingBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT))
  // vkcall(vkh::CreateBuffer(device, &stagingBuffers[5].first, _stagingBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT))
  // vkcall(vkh::CreateBuffer(device, &stagingBuffers[6].first, _stagingBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT))


  for(int i = 0; i < 7; ++i){
    VkMemoryRequirements req{};
    vkcall(vkh::CreateBuffer(device, &stagingBuffers[i].first, _stagingBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT))
    vkGetBufferMemoryRequirements(device, stagingBuffers[i].first, &req);
    vkcall(MemoryVK::Allocate(device, &stagingBuffers[i].second, req.size, _macosHostAccessFlag))
    vkcall(vkBindBufferMemory(device, stagingBuffers[i].first, stagingBuffers[i].second, 0))
  }

  VkMemoryRequirements req{};
  // vkGetBufferMemoryRequirements(device, stagingBuffers[0].first, &req);
  //
  // vkcall(MemoryVK::Allocate(device, &stagingBuffers[0].second, req.size, _macosHostAccessFlag))
  // vkcall(vkBindBufferMemory(device, stagingBuffers[0].first, stagingBuffers[0].second, 0))
  //
  // vkGetBufferMemoryRequirements(device, stagingBuffers[1].first, &req);
  // vkcall(MemoryVK::Allocate(device, &stagingBuffers[1].second, req.size, _macosHostAccessFlag))
  // vkcall(vkBindBufferMemory(device, stagingBuffers[1].first, stagingBuffers[1].second, 0))
  
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


  glm::mat4 x = glm::mat4(1);
  memcpy(DefaultGPassStub.transform, &x, sizeof(glm::mat4));

  x = glm::lookAt(glm::vec3(0.0f, -2.0f, -4.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
  memcpy(DefaultGPassStub.view, &x, sizeof(glm::mat4));

  x = glm::perspectiveFov(glm::radians(60.0f), static_cast<float>(swapchainExtent.width), static_cast<float>(swapchainExtent.height), 0.4f, 100.0f);
  memcpy(DefaultGPassStub.projection, &x, sizeof(glm::mat4));


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

  VkClearValue col[2]{
    // {0.1f, 0.1f, 0.1f, 1.0f},
    {0.4f, 0.5f, 0.6f, 1.0f},
    {1.0, 1.0, 1.0, 1.0}
  };

// gbufferpass
  VkRenderPassBeginInfo rpass{};
  rpass.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  rpass.pNext = nullptr;
  rpass.renderPass = mainRenderpass;
  rpass.renderArea = VkRect2D{{0,0}, {swapchainExtent.width,swapchainExtent.height}};
  rpass.clearValueCount = 2;
  rpass.pClearValues = col;
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

  //draw list execute
  for(const GBufEntry& e: drawList){


    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(mainCommandBuffer, 0, 1, &e.vertex->handle, offsets);
    vkCmdBindIndexBuffer(mainCommandBuffer, e.index->handle, 0, VK_INDEX_TYPE_UINT16);

    vkCmdBindDescriptorSets(mainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 
    geoPassPipelineLayout, 0, 1, &e.texture->descriptor, 0, nullptr);
    


    vkCmdPushConstants(mainCommandBuffer, geoPassPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GeometryPassPush), e.push);
    vkCmdDrawIndexed(mainCommandBuffer, e.numIndices, 1 ,0 ,0, 0);
  }

  vkCmdBindPipeline(mainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, skyboxPipeline);

    vkCmdBindDescriptorSets(mainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 
    skyboxPipelineLayout, 0, 1, &skyboxDescriptorSet, 0, nullptr);

  vkCmdDraw(mainCommandBuffer, 3, 1 ,0 ,0);

  vkCmdEndRenderPass(mainCommandBuffer);
  vkEndCommandBuffer(mainCommandBuffer);

  drawList.clear();

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
    vkQueueWaitIdle(graphicQueue);
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

  vkcall(vkQueueSubmit(graphicQueue, 1, &submitInfo, VK_NULL_HANDLE))
  vkcall(vkQueueWaitIdle(graphicQueue))

  //texture 
  std::string texpath(_SSF_GENERATED_TEXTURE_FOLDER);;
  ssf::core::ImageData image  = ssf::core::LoadImage(texpath.append("404.png").c_str());
  ssf::core::UnloadImage(image);

  VkExtent3D exent{static_cast<uint32_t>(image.width), static_cast<uint32_t>(image.height), 1};

  vkcall(ivk::wrappers::CreateImage(device, VK_FORMAT_R8G8B8A8_SRGB, exent, VK_IMAGE_TYPE_2D,1,
  VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_SAMPLE_COUNT_1_BIT,
  VK_IMAGE_TILING_OPTIMAL, 1, 0, &_TmpCube.texture))

  VkMemoryRequirements textureReq{};
  vkGetImageMemoryRequirements(device, _TmpCube.texture, &textureReq);
  vkcall(MemoryVK::Allocate(device, &_TmpCube.texHandle, textureReq.size, _macosDeviceLocalFlag))
  vkcall(vkBindImageMemory(device, _TmpCube.texture, _TmpCube.texHandle, 0))

  VkComponentMapping mappings{ VK_COMPONENT_SWIZZLE_R,
    VK_COMPONENT_SWIZZLE_G,
    VK_COMPONENT_SWIZZLE_B,
    VK_COMPONENT_SWIZZLE_A
  };

  VkImageSubresourceRange range{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
  ivk::wrappers::CreateImageView2D(device, _TmpCube.texture, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_VIEW_TYPE_2D, mappings, range, &_TmpCube.textureView);


  ivk::TransitionImageLayoutData transtionData{
    _TmpCube.texture,
    VK_ACCESS_NONE, VK_ACCESS_TRANSFER_WRITE_BIT,
    VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    range
  };
  
  vkResetCommandBuffer(mainCommandBuffer, 0);

  ivk::wrappers::BeginCommandBuffer(mainCommandBuffer);
  ivk::TransitionImageLayoutsOp(mainCommandBuffer, &transtionData, 1, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

  vkcall(vkMapMemory(device, stagingBuffers[0].second, 0, image.bytes, 0, &data))
  memcpy(data, image.data, image.bytes);
  vkUnmapMemory(device, stagingBuffers[0].second);

  VkBufferImageCopy c{};
  c.bufferImageHeight = 0;
  c.bufferOffset = 0;
  c.bufferRowLength = 0;
  c.imageOffset = {0,0,0};
  c.imageExtent = exent;
  c.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  c.imageSubresource.baseArrayLayer = 0;
  c.imageSubresource.layerCount = 1;
  c.imageSubresource.mipLevel = 0;
  vkCmdCopyBufferToImage(mainCommandBuffer, stagingBuffers[0].first, _TmpCube.texture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &c);
  
  transtionData.prevAccess = VK_ACCESS_TRANSFER_WRITE_BIT;
  transtionData.nextAccess = VK_ACCESS_SHADER_READ_BIT;
  transtionData.prevLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  transtionData.nextLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  ivk::TransitionImageLayoutsOp(mainCommandBuffer, &transtionData, 1, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
  ivk::wrappers::EndCommandBuffer(mainCommandBuffer);

  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.pNext = nullptr;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &mainCommandBuffer;

  vkcall(vkQueueSubmit(graphicQueue, 1, &submitInfo, VK_NULL_HANDLE))
  vkcall(vkQueueWaitIdle(graphicQueue))

  VkDescriptorImageInfo imageInfo{};
  imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  imageInfo.imageView = _TmpCube.textureView;
  imageInfo.sampler = fiSamplers[Sampler::ClampTexture];

  for (size_t i = 0; i < 2; i++) {

    VkWriteDescriptorSet s{};
    s.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    s.pNext = 0;
    s.dstSet = textureDescSet[i];
    s.dstBinding = 0;
    s.dstArrayElement = 0;
    s.descriptorCount = 1;
    s.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    s.pImageInfo = &imageInfo;
    s.pBufferInfo = nullptr;
    s.pTexelBufferView = nullptr;
    vkUpdateDescriptorSets(device, 1, &s, 0, nullptr);
}

  //constantbuffer
  glm::mat4 x = glm::mat4(1);
  memcpy(DefaultGPassStub.transform, &x, sizeof(glm::mat4));

  x = glm::lookAt(glm::vec3(0.0f, -2.0f, -4.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
  memcpy(DefaultGPassStub.view, &x, sizeof(glm::mat4));
  x = glm::perspectiveFov(glm::radians(60.0f), 
                                          static_cast<float>(swapchainExtent.width),
                                          static_cast<float>(swapchainExtent.height), 
                                          0.4f, 100.0f);
  memcpy(DefaultGPassStub.projection, &x, sizeof(glm::mat4));
}

  void VK::CreateFixedSamplers(bool rebuild){
    if(rebuild){ ivk::wrappers::DestroySamplers(device, fiSamplers.data(), fiSamplers.size(), nullptr);}

    vkcall(ivk::wrappers::CreateSampler(device, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
                                        true, 16, 0, 0, nullptr, &fiSamplers[Sampler::ClampTexture]))
  }

  //TODO:Figure out a better pre allocation stategy.
  void VK::CreateFixedDescriptors(){
    VkDescriptorPoolSize combinedSamplePool{};
    combinedSamplePool.descriptorCount = 20;
    combinedSamplePool.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    VkDescriptorPoolSize uniformBufferPool{};
    uniformBufferPool.descriptorCount = 10;
    uniformBufferPool.type =  VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

    // etc for the rest of types

    VkDescriptorPoolSize dpsArr[2]{
      combinedSamplePool,
      uniformBufferPool,
    };

    VkDescriptorPoolCreateInfo cDescriptorPool{};
    cDescriptorPool.flags = 0;
    cDescriptorPool.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    cDescriptorPool.pNext = nullptr;
    cDescriptorPool.poolSizeCount = 2;
    cDescriptorPool.pPoolSizes = dpsArr;
    cDescriptorPool.maxSets = 10;
    vkcall(vkCreateDescriptorPool(device, &cDescriptorPool, nullptr, &geoPassDescriptorPool))

  }

  void VK::GpuUploadBufData(VkCommandBuffer cmdBuf, VkDeviceMemory stage, VkBuffer srcBuf, VkBuffer dstBuf, const void* const pData, size_t bytes){
    void* mem;
    vkcall(vkMapMemory(device, stage, 0, VK_WHOLE_SIZE, 0, &mem))
    memcpy(mem, pData, bytes);
    vkUnmapMemory(device, stage);


    VkBufferCopy region{};
    region.size = bytes;
    vkCmdCopyBuffer(cmdBuf, srcBuf, dstBuf, 1, &region);
  }

  
  void VK::GpuUploadImageData(VkCommandBuffer cmdBuf,VkExtent3D extent, VkBufferImageCopy& copy, VkDeviceMemory stage, VkBuffer srcBuf, VkImage dstBuf, 
  const void* const pData){
    void* mem;
    vkcall(vkMapMemory(device, stage, 0, VK_WHOLE_SIZE, 0, &mem))
    memcpy(mem, pData, (extent.width * extent.height * 4));
    vkUnmapMemory(device, stage);

    VkBufferImageCopy c{};
    c.bufferImageHeight = 0;
    c.bufferOffset = 0;
    c.bufferRowLength = 0;
    c.imageOffset = {0,0,0};
    c.imageExtent = extent;
    c.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    c.imageSubresource.baseArrayLayer = 0;
    c.imageSubresource.layerCount = 1;
    c.imageSubresource.mipLevel = 0;

    vkCmdCopyBufferToImage(mainCommandBuffer, srcBuf, dstBuf, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);
  }

  void VK::SetGpuImageBarriers(VkCommandBuffer cmdBuf, const VkImageMemoryBarrier* const pBarrier, uint32_t count, 
                               VkPipelineStageFlags src, VkPipelineStageFlags dst){
    vkCmdPipelineBarrier(cmdBuf, src, dst , 0, 
    0, nullptr, //VkMemoryBarrier
    0, nullptr, //VkBufferMemoryBarrier
    count, pBarrier);// VkImageMemoryBarrier
  }

  // void VK::GpuUploadImageData(VkDevice device, VkDeviceMemory staged, const void* const pData, size_t bytes){ }

  VK::GeoHandle VK::CreateGeometry(const GeometryData& geo){
    VkMemoryRequirements memReq;
    VkBuffer buf;
    VkDeviceMemory handle;
    VkImage tex;
    VkImageView view;
    GBufEntry entry{};
    VkDescriptorSet texelSet;

    VkSubmitInfo submit{};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.pNext = nullptr;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &mainCommandBuffer;

    vkResetCommandBuffer(mainCommandBuffer, 0);
    vkcall(ivk::wrappers::BeginCommandBuffer(mainCommandBuffer))

    //create vertext buffer
    vkcall(vkh::CreateBuffer(device, &buf, geo.vertexBytes, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT))
    vkGetBufferMemoryRequirements(device, buf, &memReq);
    vkcall(MemoryVK::Allocate(device, &handle, memReq.size, _macosDeviceLocalFlag))
    vkcall(vkBindBufferMemory(device, buf, handle, 0))
    bufferList.push_back(GpuBuffer{buf, handle});
    entry.vertex = --bufferList.end();

    GpuUploadBufData(mainCommandBuffer, stagingBuffers[0].second, stagingBuffers[0].first, buf, geo.pVertex, geo.vertexBytes);
    vkcall(ivk::wrappers::EndCommandBuffer(mainCommandBuffer))
    vkQueueSubmit(graphicQueue, 1, &submit, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicQueue);

    //create index buffer
    vkResetCommandBuffer(mainCommandBuffer, 0);
    vkcall(ivk::wrappers::BeginCommandBuffer(mainCommandBuffer))

    vkcall(vkh::CreateBuffer(device, &buf, geo.indicesBytes, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT))
    vkGetBufferMemoryRequirements(device, buf, &memReq);
    vkcall(MemoryVK::Allocate(device, &handle, memReq.size, _macosDeviceLocalFlag))
    vkcall(vkBindBufferMemory(device, buf, handle, 0))
    GpuUploadBufData(mainCommandBuffer, stagingBuffers[0].second, stagingBuffers[0].first, buf, geo.pIndices, geo.indicesBytes);
    bufferList.push_back(GpuBuffer{buf, handle});
    entry.index = --bufferList.end();

    vkcall(ivk::wrappers::EndCommandBuffer(mainCommandBuffer))
    vkQueueSubmit(graphicQueue, 1, &submit, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicQueue);
    
    //create texture
    vkResetCommandBuffer(mainCommandBuffer, 0);
    vkcall(ivk::wrappers::BeginCommandBuffer(mainCommandBuffer))

    vkcall(ivk::wrappers::CreateImage(device, VK_FORMAT_R8G8B8A8_SRGB, VkExtent3D{geo.textureWidth, geo.textureHeight, 1},
    VK_IMAGE_TYPE_2D, 1, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_SAMPLE_COUNT_1_BIT,
    VK_IMAGE_TILING_OPTIMAL, 1, 0, &tex))

    vkGetImageMemoryRequirements(device, tex, &memReq);
    vkcall(MemoryVK::Allocate(device, &handle, memReq.size, _macosDeviceLocalFlag))
    vkcall(vkBindImageMemory(device, tex, handle, 0))

    VkImageSubresourceRange range{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    vkcall(ivk::wrappers::CreateImageView2D(device, tex, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_VIEW_TYPE_2D, defaultTextureCMapping, range, &view))

    texelList.push_back(GpuTexel{tex, handle, view});
    entry.texture = --texelList.end();

    auto transfer = ivk::wrappers::CreateImageMemoryBarrier(device, tex, 0, 0, VK_ACCESS_NONE, VK_ACCESS_TRANSFER_WRITE_BIT,
                     VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, range);

    auto shaderReady = ivk::wrappers::CreateImageMemoryBarrier(device, tex, 0, 0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, range);

    SetGpuImageBarriers(mainCommandBuffer, &transfer, 1, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

    VkBufferImageCopy c{};
    c.bufferImageHeight = 0;
    c.bufferOffset = 0;
    c.bufferRowLength = 0;
    c.imageOffset = {0,0,0};
    c.imageExtent = VkExtent3D{geo.textureWidth, geo.textureHeight, 1};
    c.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    c.imageSubresource.baseArrayLayer = 0;
    c.imageSubresource.layerCount = 1;
    c.imageSubresource.mipLevel = 0;

    GpuUploadImageData(mainCommandBuffer, VkExtent3D{geo.textureWidth, geo.textureHeight, 1}, c, stagingBuffers[0].second, 
    stagingBuffers[0].first, tex, geo.texture);

    SetGpuImageBarriers(mainCommandBuffer, &shaderReady, 1, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

    vkcall(ivk::wrappers::EndCommandBuffer(mainCommandBuffer))
    vkQueueSubmit(graphicQueue, 1, &submit, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicQueue);
    vkResetCommandBuffer(mainCommandBuffer, 0);

    VkDescriptorSetAllocateInfo dsai{};
    dsai.pSetLayouts = geoPassDescriptorLayout;
    dsai.descriptorPool = textureDP[0].pool;
    dsai.descriptorSetCount = 1;
    dsai.pNext = nullptr;
    dsai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    vkAllocateDescriptorSets(device,&dsai , &texelSet);

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = view;
    imageInfo.sampler = fiSamplers[Sampler::ClampTexture];

    VkWriteDescriptorSet s{};
    s.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    s.pNext = 0;
    s.dstSet = texelSet;
    s.dstBinding = 0;
    s.dstArrayElement = 0;
    s.descriptorCount = 1;
    s.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    s.pImageInfo = &imageInfo;
    s.pBufferInfo = nullptr;
    s.pTexelBufferView = nullptr;
    vkUpdateDescriptorSets(device, 1, &s, 0, nullptr);

    entry.texture->descriptor = texelSet;
    entry.push = &DefaultGPassStub;


    entry.numIndices = geo.numIndices;
    geometryList.push_back(entry);
   return --geometryList.end();
  }

  void VK::DestroyGeometry(GeoHandle& geometry){

  }

  void VK::MapGeometryPassPushBuf(GeoHandle& handle, void* pData){
    handle->push = pData;
  }

  void VK::UnmapGeometryPassPushBuf(GeoHandle& handle){
    handle->push = &DefaultGPassStub;
  }

  void VK::AddToDrawList(GeoHandle& geometry){
    drawList.push_back(*geometry);
  }

  void VK::RemoveToDrawList(GeoHandle& geometry){}


  void VK::SetSkyBox(ResourceHandle cubemap){

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = static_cast<GpuCubeMap*>(cubemap)->view;
    imageInfo.sampler = fiSamplers[Sampler::ClampTexture];

    VkWriteDescriptorSet s{};
    s.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    s.pNext = 0;
    s.dstSet = skyboxDescriptorSet;
    s.dstBinding = 0;
    s.dstArrayElement = 0;
    s.descriptorCount = 1;
    s.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    s.pImageInfo = &imageInfo;
    s.pBufferInfo = nullptr;
    s.pTexelBufferView = nullptr;

    vkUpdateDescriptorSets(device, 1, &s, 0, nullptr);

  };

  void VK::tCreateDescriptorPools(DescriptorPoolType type, uint32_t count, DescriptorPool* pMemory){
    const int kMaxSets = 20;
    VkDescriptorPoolCreateInfo cDescriptorPool{};

    cDescriptorPool.pNext = nullptr;
    cDescriptorPool.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    cDescriptorPool.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    cDescriptorPool.maxSets = kMaxSets;

    switch (type){
      case DescriptorPoolTexture:
        const int kMaxTextureDescriptorCount = 20;
        VkDescriptorPoolSize cs{};
        cs.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        cs.descriptorCount = kMaxTextureDescriptorCount;
        cDescriptorPool.pPoolSizes = &cs;
        cDescriptorPool.poolSizeCount = 1;
        break;
    }

    
    for(int i = 0; i < count; ++i){
      vkcall(vkCreateDescriptorPool(device, &cDescriptorPool, nullptr, &pMemory[i].pool))
      pMemory[i].maxSets = kMaxSets;
      pMemory[i].remainingSets = kMaxSets;
    }
  }

ResourceHandle VK::CreateCubeMap(uint32_t size){
  VkImageView v;
  VkImage image;
  VkMemoryRequirements requirements;
  VkDeviceMemory memory;

  vkcall(ivk::wrappers::CreateImage(device, VK_FORMAT_R8G8B8A8_SRGB, VkExtent3D{size, size, 1}, VK_IMAGE_TYPE_2D, 1, 
  VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_SAMPLE_COUNT_1_BIT,
  VK_IMAGE_TILING_OPTIMAL, 6, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT, &image))
   
  VkImageSubresourceRange sr;
  sr.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  sr.baseArrayLayer = 0;
  sr.baseMipLevel = 0;
  sr.layerCount = 6;
  sr.levelCount = 1;
  
  vkGetImageMemoryRequirements(device, image, &requirements);
  vkcall(MemoryVK::Allocate(device, &memory, requirements.size, _macosDeviceLocalFlag))
  vkBindImageMemory(device, image, memory, 0);
  vkcall(ivk::wrappers::CreateImageView2D(device, image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_VIEW_TYPE_CUBE, defaultTextureCMapping, sr, &v))

  ImageResource* resource = &imageLUT.construct();
  GpuCubeMap* ret = static_cast<GpuCubeMap*>(resourceLUT.construct());

  *resource = ImageResource{ image, memory, VkExtent3D{size, size, 1}, sr};
  *ret  = GpuCubeMap{v, resource};
  return ret;
}

  void VK::WriteCubeMap(ResourceHandle handle, const CubeMapWriteDescription& desc){
    
    GpuCubeMap* h = static_cast<GpuCubeMap*>(handle);

    auto transfer = ivk::wrappers::CreateImageMemoryBarrier(device, h->resource->image, 0, 0, VK_ACCESS_NONE, VK_ACCESS_TRANSFER_WRITE_BIT,
                     VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, h->resource->range);

    auto shaderReady = ivk::wrappers::CreateImageMemoryBarrier(device, h->resource->image, 0, 0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, h->resource->range);

    vkcall(ivk::wrappers::BeginCommandBuffer(mainCommandBuffer))

    SetGpuImageBarriers(mainCommandBuffer, &transfer, 1, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

    VkBufferImageCopy c{};
    c.bufferImageHeight = 0;
    c.bufferOffset = 0;
    c.bufferRowLength = 0;
    c.imageOffset = {0,0,0};
    c.imageExtent = h->resource->extent;
    c.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    c.imageSubresource.layerCount = 1;
    c.imageSubresource.mipLevel = 0;

    for(int i = 0; i < 6; ++i){
    c.imageSubresource.baseArrayLayer = i;
      GpuUploadImageData(mainCommandBuffer, h->resource->extent, c, stagingBuffers[i].second, stagingBuffers[i].first, h->resource->image, desc.data[i]);
    }

    SetGpuImageBarriers(mainCommandBuffer, &shaderReady, 1, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

    vkcall(ivk::wrappers::EndCommandBuffer(mainCommandBuffer))

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &mainCommandBuffer;

    vkcall(vkQueueSubmit(graphicQueue, 1, &submitInfo, 0))
    vkcall(vkQueueWaitIdle(graphicQueue))
    vkcall(vkResetCommandBuffer(mainCommandBuffer, 0))
  }

  void VK::DestroyCubeMap(ResourceHandle handle){
    GpuCubeMap* h = static_cast<GpuCubeMap*>(handle);
    vkDestroyImage(device, h->resource->image, nullptr);
    vkDestroyImageView(device, h->view, nullptr);
    MemoryVK::Deallocate(device, h->resource->memory, nullptr);
  }


void VK::Destroy(){
  vkDestroyBuffer(device, _TmpCube.ibo, nullptr);
  vkDestroyBuffer(device, _TmpCube.vbo, nullptr);

  vkDestroyDevice(device, nullptr);
  vkDestroyInstance(instance, nullptr);
}
