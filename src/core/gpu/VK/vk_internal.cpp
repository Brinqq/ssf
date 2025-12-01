#include "vk_internal.h"

#include <bcl/containers/vector.h>

#include <fstream>
#include <assert.h>

namespace ivk{

VkResult CreateGraphicPipeline(VkDevice device, const PipelineShaders& shaders, VkRenderPass renderpass, VkPipeline* pipeline){
  //TODO: Shaders reflect and set layout requirements instead of hardcoding
  
  //shader stage
  VkPipelineShaderStageCreateInfo cShaderStage[2]{};
  
  cShaderStage[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  cShaderStage[0].pNext = nullptr;
  cShaderStage[0].flags = 0;
  cShaderStage[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
  cShaderStage[0].module = shaders.vertex;
  cShaderStage[0].pName = "main";
  cShaderStage[0].pSpecializationInfo = nullptr;

  cShaderStage[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  cShaderStage[1].pNext = nullptr;
  cShaderStage[1].flags = 0;
  cShaderStage[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  cShaderStage[1].module = shaders.pixel;
  cShaderStage[1].pName = "main";
  cShaderStage[1].pSpecializationInfo = nullptr;

  //vertex layout
  VkPipelineVertexInputStateCreateInfo cVertexInputState{};
  cVertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  cVertexInputState.pNext = nullptr;
  cVertexInputState.flags = 0;

  VkVertexInputBindingDescription vbd;
  vbd.binding = 0;
  vbd.stride = sizeof(float) * 8;
  vbd.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

  cVertexInputState.vertexBindingDescriptionCount = 1;  
  cVertexInputState.pVertexBindingDescriptions = &vbd;

  VkVertexInputAttributeDescription vad[3];
  vad[0].location = 0;
  vad[0].binding = 0;
  vad[0].format = VK_FORMAT_R32G32B32_SFLOAT;
  vad[0].offset = 0;

  vad[1].location = 1;
  vad[1].binding = 0;
  vad[1].format = VK_FORMAT_R32G32_SFLOAT;
  vad[1].offset = sizeof(float) * 3;

  vad[2].location = 2;
  vad[2].binding = 0;
  vad[2].format = VK_FORMAT_R32G32B32_SFLOAT;
  vad[2].offset = sizeof(float) * 5;


  cVertexInputState.vertexAttributeDescriptionCount = 3;
  cVertexInputState.pVertexAttributeDescriptions = vad;

  //input stage
  VkPipelineInputAssemblyStateCreateInfo cInputAssemblyState{};
  cInputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  cInputAssemblyState.pNext = nullptr;
  cInputAssemblyState.flags = 0;
  cInputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  cInputAssemblyState.primitiveRestartEnable = false;

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
  viewport.width = 3024;
  viewport.height = 1844;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  cViewportState.viewportCount = 1;
  cViewportState.pViewports = &viewport;

  VkRect2D scissor{};
  scissor.offset.x = 0;
  scissor.offset.y = 0;
  scissor.extent.width = 3024;
  scissor.extent.height = 1844;
  cViewportState.scissorCount = 1;
  cViewportState.pScissors = &scissor;
  
  VkPipelineRasterizationStateCreateInfo cRasterizationState{};
  cRasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  cRasterizationState.pNext = nullptr;
  cRasterizationState.flags = 0;
  cRasterizationState.depthClampEnable = VK_FALSE;
  cRasterizationState.rasterizerDiscardEnable = VK_FALSE;
  cRasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
  cRasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
  cRasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE;
  cRasterizationState.depthBiasEnable = VK_FALSE;
  //wtf is this 
  cRasterizationState.depthBiasConstantFactor = 0.0f;
  cRasterizationState.depthBiasClamp = 0.0f;
  cRasterizationState.depthBiasSlopeFactor = 0.0f;
  cRasterizationState.lineWidth = 1.0f;
  //
  
  VkPipelineMultisampleStateCreateInfo cMultisampleState{};
  cMultisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  cMultisampleState.pNext = nullptr;
  cMultisampleState.flags = 0;
  cMultisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  cMultisampleState.sampleShadingEnable = VK_FALSE;

  VkPipelineDepthStencilStateCreateInfo cDepthStencilState{};
  cDepthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  cDepthStencilState.pNext = nullptr;
  cDepthStencilState.flags = 0;
  cDepthStencilState.depthTestEnable = VK_TRUE;
  cDepthStencilState.depthWriteEnable = VK_TRUE;
  cDepthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
  cDepthStencilState.depthBoundsTestEnable = VK_FALSE;
  cDepthStencilState.stencilTestEnable = VK_FALSE;
  cDepthStencilState.minDepthBounds = 0.0f;
  cDepthStencilState.maxDepthBounds = 1.0f;
  cDepthStencilState.front = {};
  cDepthStencilState.back = {};


  VkPipelineColorBlendAttachmentState colorBlendAttachment{};
  colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  colorBlendAttachment.blendEnable = VK_FALSE;
  colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
  colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

  VkPipelineColorBlendStateCreateInfo colorBlending{};
  colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlending.logicOpEnable = VK_FALSE;
  colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
  colorBlending.attachmentCount = 1;
  colorBlending.pAttachments = &colorBlendAttachment;
  colorBlending.blendConstants[0] = 0.0f; // Optional
  colorBlending.blendConstants[1] = 0.0f; // Optional
  colorBlending.blendConstants[2] = 0.0f; // Optional
  colorBlending.blendConstants[3] = 0.0f; // Optional

  VkPipelineTessellationStateCreateInfo cTessellationState{};

  VkGraphicsPipelineCreateInfo cGraphicsPipeline{};
  cGraphicsPipeline.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO; 
  cGraphicsPipeline.pNext = nullptr;
  cGraphicsPipeline.flags = 0;

  //filled automaticly with shader reflection
  // cGraphicsPipeline.pStages = &cShaderStage;
  cGraphicsPipeline.stageCount = 2;
  cGraphicsPipeline.pStages = cShaderStage;
  cGraphicsPipeline.pVertexInputState = &cVertexInputState;
  cGraphicsPipeline.pTessellationState = nullptr;
  cGraphicsPipeline.pInputAssemblyState = &cInputAssemblyState;
  cGraphicsPipeline.layout = shaders.layout;

  //dynamic state some static others enabled with param flag
  cGraphicsPipeline.pViewportState = &cViewportState;
  cGraphicsPipeline.pDynamicState = &cDynamicState;

  //idk
  cGraphicsPipeline.pMultisampleState = &cMultisampleState;

  //static always enabled customized through flags
  cGraphicsPipeline.pRasterizationState = &cRasterizationState;
  cGraphicsPipeline.pDepthStencilState = &cDepthStencilState;
  cGraphicsPipeline.pColorBlendState = &colorBlending;

  //user defined with parameters to create function
  cGraphicsPipeline.renderPass = renderpass;
  cGraphicsPipeline.subpass = 0;

  return vkCreateGraphicsPipelines(device, nullptr, 1, &cGraphicsPipeline, nullptr, pipeline);
}


void CopyBuffers(VkDevice device, VkCommandBuffer buf, CopyBufferOp* pCopyOp, uint32_t count){

  VkCommandBufferBeginInfo begin{};
  begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  vkBeginCommandBuffer(buf, &begin);

  VkBufferCopy region{};

  for(int i = 0; i < count; ++i){
    region.srcOffset = pCopyOp[i].srcOffset;
    region.dstOffset = pCopyOp[i].dstOffset;
    region.size = pCopyOp[i].bytes;
    vkCmdCopyBuffer(buf, pCopyOp[i].src, pCopyOp[i].dst, 1, &region);
  }

  vkEndCommandBuffer(buf);
}


void TransitionImageLayoutsOp(VkCommandBuffer buf, TransitionImageLayoutData* pData, uint32_t count,
                             VkPipelineStageFlags prev, VkPipelineStageFlags next){
  bcl::small_vector<VkImageMemoryBarrier, 5> arr;

  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;;
  barrier.pNext = nullptr;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

  for(int i = 0; i < count; ++i){
    barrier.srcAccessMask = pData[i].prevAccess;
    barrier.dstAccessMask = pData[i].nextAccess;
    barrier.oldLayout = pData[i].prevLayout;
    barrier.newLayout = pData[i].nextLayout;;
    barrier.subresourceRange = pData[i].range;
    barrier.image = pData[i].image;
    arr.push_back(barrier);
  }

  vkCmdPipelineBarrier(buf, prev, next, 0, 0, nullptr, 0, nullptr, arr.size(), arr.data());
}

//utils
std::pair<void*, size_t> CompileShaderSource(const char* path){
  std::pair<void*, size_t> ret{nullptr, 0};

  std::ifstream stream{};
  stream.open(path, std::ios::ate | std::ios::binary);
  
  if(!stream.is_open()){
    assert(0);
    return ret;
  }

  size_t bytes = stream.tellg();
  stream.seekg(0);
  ret.first = malloc(bytes);
  stream.read(static_cast<char*>(ret.first), bytes);
  stream.close();
  ret.second = bytes;
  return ret;
}

}
