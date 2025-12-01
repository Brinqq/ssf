#include "vk_helper.h"
#include "vk_debug.h"

#include "core/debug.h"

#include <alloca.h>
#include <vector>

#include "GLFW/glfw3.h"
#if __APPLE__

  VkSurfaceKHR vkh::GetPlatformSurface(VkInstance instance, GLFWwindow* handle){
    VkSurfaceKHR ret;
    vkcall(glfwCreateWindowSurface(instance, handle, nullptr, &ret));
    return ret;
  }

  void vkh::GetPlatformExtensions(bcl::small_vector<const char*>& ext){
    if(!glfwInit()){
      ssf_runtime_error();
    };
    uint32_t count;
   const char** ppExtensions = glfwGetRequiredInstanceExtensions(&count);
   for(int i = 0; i < count; ++i){
    ext.push_back(ppExtensions[i]);
   }
    glfwTerminate();
  }

#endif


#if _WIN32

#endif

#if __LINUX__

#endif

// FIXME: These are hardcoded for now. add support for dynamically obtaining valid surface characteristics.
// only guaranteed work on apple sillicon.

VkFormat vkh::GetCompatibleSurfaceFormat(VkPhysicalDevice gpu, VkSurfaceKHR surface){
  return VK_FORMAT_B8G8R8A8_UNORM;
}

VkColorSpaceKHR vkh::GetCompatibleSurfaceColorSpace(VkPhysicalDevice gpu, VkSurfaceKHR surface){
  return VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
}

VkExtent2D vkh::GetCompatibleSurfaceExtent(){
  return VkExtent2D{1920, 1080};
}
//------------------------------------------------
//fixme end




VkPhysicalDevice vkh::GetGpu(VkInstance& instance){
  //FIXME:(Crossplatform support) add gpu selection only selects first gpu at the moment.

  uint32_t count = 0;
  vkEnumeratePhysicalDevices(instance, &count, nullptr);
  VkPhysicalDevice* pDat = (VkPhysicalDevice*)alloca(count * sizeof(VkPhysicalDevice));
  vkEnumeratePhysicalDevices(instance, &count, pDat);
  return pDat[0];

}

void vkh::GenerateQueueFamilies(VkPhysicalDevice device, QueueFamily* const pDat, size_t& count){
  if(pDat != nullptr){
    uint32_t lcount = count;
    VkQueueFamilyProperties* pProperties = (VkQueueFamilyProperties*)alloca(lcount * sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(device, &lcount, pProperties);

    for(int i = 0; i < lcount; ++i){
      pDat[i].index = i;
      pDat[i].maxQueues = pProperties[i].queueCount;
      pDat[i].bits = QueueBitNone;
      if(pProperties->queueFlags & VK_QUEUE_GRAPHICS_BIT){ pDat[i].bits = static_cast<QueueBitTypes>(pDat[i].bits | QueueBitGraphic);}
      if(pProperties->queueFlags & VK_QUEUE_COMPUTE_BIT){ pDat[i].bits = static_cast<QueueBitTypes>(pDat[i].bits  | QueueBitCompute);}
      if(pProperties->queueFlags & VK_QUEUE_TRANSFER_BIT){ pDat[i].bits = static_cast<QueueBitTypes>(pDat[i].bits | QueueBitTransfer);}
      if(pProperties->queueFlags & VK_QUEUE_SPARSE_BINDING_BIT){ pDat[i].bits = static_cast<QueueBitTypes>(pDat[i].bits | QueueBitSparse);}
    }

    return;
  }

  uint32_t lcount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &lcount, nullptr);
  count = lcount;
}

VkDeviceQueueCreateInfo vkh::CreateDeviceQueueCI(uint32_t index, uint32_t count, float p){
  VkDeviceQueueCreateInfo ret{};
  ret.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  ret.flags = 0;
  ret.queueCount = count;
  ret.queueFamilyIndex = index;
  ret.pQueuePriorities = &p;
  return ret;
}

VkPipeline vkh::CreatePipeline(VkDevice device, vkh::PipelineState& pipeline){

  //shader stage
  VkPipelineShaderStageCreateInfo cShaderStage{};
  // cShaderStage
  // cGraphicPipeline.stageCount = 0;
  // cGraphicPipeline.pStages;

  //vertex layout
  VkPipelineVertexInputStateCreateInfo cVertexInputState{};
  cVertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  cVertexInputState.pNext = nullptr;
  cVertexInputState.flags = 0;

  VkVertexInputBindingDescription vbd{};
  vbd.binding = 0;
  vbd.stride = sizeof(float) * 3;
  vbd.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

  cVertexInputState.vertexBindingDescriptionCount = 1;  
  cVertexInputState.pVertexBindingDescriptions = &vbd;

  VkVertexInputAttributeDescription vad{};
  vad.location = 0;
  vad.binding = 0;
  vad.format = VK_FORMAT_R32G32B32_SFLOAT;
  vad.offset = 0;

  cVertexInputState.vertexAttributeDescriptionCount = 1;
  cVertexInputState.pVertexAttributeDescriptions = &vad;

  //input stage
  VkPipelineInputAssemblyStateCreateInfo cInputAssemblyState{};
  cInputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  cInputAssemblyState.pNext = nullptr;
  cInputAssemblyState.flags = 0;
  cInputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  cInputAssemblyState.primitiveRestartEnable = false;
  return 0;

  //dynamic state
  VkPipelineDynamicStateCreateInfo cDynamicState{};
  cDynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  cDynamicState.pNext = nullptr;
  cDynamicState.flags = 0;

  VkDynamicState dynamicStates[2]{VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
  cDynamicState.dynamicStateCount = 2;
  cDynamicState.pDynamicStates = dynamicStates;

  // Viewport
  VkPipelineViewportStateCreateInfo cViewportState{};
  cViewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  cViewportState.pNext = nullptr;
  cViewportState.flags = 0;

  VkViewport viewport{};
  viewport.x = 0;
  viewport.y = 0;
  viewport.width = 1920;
  viewport.height = 1080;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  cViewportState.viewportCount = 1;
  cViewportState.pViewports = &viewport;

  VkRect2D scissor{};
  scissor.offset.x = 0;
  scissor.offset.y = 0;
  scissor.extent.width = 1920;
  scissor.extent.height = 1080;
  cViewportState.scissorCount = 0;
  cViewportState.pScissors = &scissor;
  
  VkPipelineRasterizationStateCreateInfo cRasterizationState{};
  cRasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  cRasterizationState.pNext = nullptr;
  cRasterizationState.flags = 0;
  cRasterizationState.depthClampEnable = VK_FALSE;
  cRasterizationState.rasterizerDiscardEnable = VK_FALSE;
  cRasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
  cRasterizationState.cullMode = VK_CULL_MODE_NONE;
  cRasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  cRasterizationState.depthBiasEnable = VK_FALSE;
  //wtf is this 
  cRasterizationState.depthBiasConstantFactor = 0.0f;
  cRasterizationState.depthBiasClamp = 0.0f;
  cRasterizationState.depthBiasSlopeFactor = 0.0f;
  cRasterizationState.lineWidth = 0.0f;
  //
  
  VkPipelineMultisampleStateCreateInfo cMultisampleState{};
  cMultisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  cMultisampleState.pNext = nullptr;
  cMultisampleState.flags = 0;

  VkPipelineDepthStencilStateCreateInfo cDepthStencilState{};
  VkPipelineColorBlendStateCreateInfo cColorBlendState{};
  VkPipelineTessellationStateCreateInfo cTessellationState{};

  VkPipelineLayoutCreateInfo cPipelineLayout{};
  VkPipelineLayout layout;
  cPipelineLayout.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

  vkCreatePipelineLayout(device, &cPipelineLayout, nullptr, &layout);

  VkGraphicsPipelineCreateInfo cGraphicsPipeline{};
  cGraphicsPipeline.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO; 
  cGraphicsPipeline.pNext = nullptr;
  cGraphicsPipeline.flags = 0;
  cGraphicsPipeline.pStages = &cShaderStage;
  cGraphicsPipeline.pVertexInputState = &cVertexInputState;
  cGraphicsPipeline.pInputAssemblyState = &cInputAssemblyState;
  cGraphicsPipeline.pTessellationState = &cTessellationState;
  cGraphicsPipeline.pViewportState = &cViewportState;
  cGraphicsPipeline.pRasterizationState = &cRasterizationState;
  cGraphicsPipeline.pMultisampleState = &cMultisampleState;
  cGraphicsPipeline.pDepthStencilState = &cDepthStencilState;
  cGraphicsPipeline.pColorBlendState = &cColorBlendState;
  cGraphicsPipeline.pDynamicState = &cDynamicState;

  cGraphicsPipeline.layout = pipeline.layout;
  cGraphicsPipeline.renderPass = pipeline.renderpass;
  cGraphicsPipeline.subpass;

  return nullptr;
}

void vkh::DestroyPipeline(VkDevice device){

}

VkRenderPass vkh::CreateRenderpass(VkDevice device){
  VkRenderPass renderpass;

  VkAttachmentDescription colorAttachment{};
  colorAttachment.flags = 0;
  colorAttachment.format = VK_FORMAT_B8G8R8A8_UNORM;
  colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentDescription depthAttachment{};
  depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  depthAttachment.initialLayout =  VK_IMAGE_LAYOUT_UNDEFINED;
  depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  depthAttachment.loadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthAttachment.format = VK_FORMAT_D32_SFLOAT;
  depthAttachment.flags = 0;

  VkAttachmentReference colorRef{};
  colorRef.attachment = 0;
  colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkAttachmentReference depthRef{};
  depthRef.attachment = 1;
  depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpassDescription{};
  subpassDescription.flags = 0;
  subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpassDescription.inputAttachmentCount = 0;
  subpassDescription.pInputAttachments = nullptr;
  subpassDescription.colorAttachmentCount = 1;
  subpassDescription.pColorAttachments = &colorRef;
  subpassDescription.preserveAttachmentCount = 0;
  subpassDescription.pPreserveAttachments = nullptr;
  subpassDescription.pResolveAttachments = nullptr;
  subpassDescription.pDepthStencilAttachment = &depthRef;

  VkSubpassDependency dependency{};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;

  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
  dependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

  VkAttachmentDescription attachments[2]{colorAttachment, depthAttachment};

  VkRenderPassCreateInfo cRenderpass{};
  cRenderpass.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  cRenderpass.pNext = nullptr;
  cRenderpass.flags = 0;
  cRenderpass.attachmentCount = 2;
  cRenderpass.pAttachments = attachments;
  cRenderpass.subpassCount = 1;
  cRenderpass.pSubpasses = &subpassDescription;
  cRenderpass.dependencyCount = 1;
  cRenderpass.pDependencies = &dependency;

  vkcall(vkCreateRenderPass(device, &cRenderpass, nullptr, &renderpass))

  return renderpass;
}

void vkh::DestroyRenderpass(VkDevice device){

}


VkImageView vkh::CreateImageView(VkDevice device, VkImage image, VkImageViewType dem, VkFormat format, VkImageAspectFlags aspect){
  VkImageView ret;
  VkImageViewCreateInfo cImageView{};
  cImageView.sType =  VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  cImageView.pNext = nullptr;
  cImageView.flags = 0;
  cImageView.image = image;
  cImageView.viewType = dem;
  cImageView.format = format;
  cImageView.components.r = VK_COMPONENT_SWIZZLE_IDENTITY,
  cImageView.components.g = VK_COMPONENT_SWIZZLE_IDENTITY,
  cImageView.components.b = VK_COMPONENT_SWIZZLE_IDENTITY,
  cImageView.components.a = VK_COMPONENT_SWIZZLE_IDENTITY,
  cImageView.subresourceRange.aspectMask = aspect;
  cImageView.subresourceRange.baseMipLevel = 0;
  cImageView.subresourceRange.levelCount = 1;
  cImageView.subresourceRange.baseArrayLayer = 0;
  cImageView.subresourceRange.layerCount = 1;
  vkcall(vkCreateImageView(device, &cImageView ,nullptr, &ret))

  return ret;
}

void vkh::DestroyImageView(VkDevice device, VkImageView view){

};

void DestroyImage(VkDevice device, VkImage* image){
  
}


VkResult vkh::CreateBuffer(VkDevice device, VkBuffer* buf, size_t bytes, const VkBufferUsageFlags usage){
  VkBufferCreateInfo cBuffer{};
    cBuffer.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    cBuffer.pNext = nullptr;
    cBuffer.flags = 0;
    cBuffer.size = bytes;
    cBuffer.usage = usage;
    cBuffer.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    cBuffer.queueFamilyIndexCount = 0 ;
    cBuffer.pQueueFamilyIndices = nullptr;
    return vkCreateBuffer(device, &cBuffer, nullptr, buf);

}


void vkh::DestroyBuffer(VkDevice device, VkBuffer buffer){
  vkDestroyBuffer(device, buffer, nullptr);
}
