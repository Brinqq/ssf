#pragma once

#include "vkdefines.h"
#include <utility>

#include <bcl/containers/span.h>

struct ShaderContainer;

namespace ivk{

struct CopyBufferOp{
  VkBuffer src;
  VkBuffer dst;
  VkDeviceSize srcOffset;
  VkDeviceSize dstOffset;
  VkDeviceSize bytes;
};

struct TransitionImageLayoutData{
  VkImage image;
  VkAccessFlags prevAccess;
  VkAccessFlags nextAccess;
  VkImageLayout prevLayout;
  VkImageLayout nextLayout;
  VkImageSubresourceRange range;
};

// Creation functions
VkResult CreateGraphicPipeline(VkDevice device, const ShaderContainer& container, VkPipelineLayout layout, VkRenderPass renderpass, VkPipeline* pipeline);

VkResult CreatePipelineLayoutFromContainer(VkDevice device, const ShaderContainer& container,
                                           const bk::span<VkDescriptorSetLayout>& sets,VkPipelineLayout* layout);

//runtime operations
//NOTE: all functions with the suffix "Op" expect for Begin and end expect a command buffer to already be in recording.
void CopyBuffers(VkDevice device, VkCommandBuffer buf, CopyBufferOp* pCopyOp, uint32_t count);

void TransitionImageLayoutsOp(VkCommandBuffer buf, TransitionImageLayoutData* pData, uint32_t count,
                             VkPipelineStageFlags prev, VkPipelineStageFlags next);


//utils
std::pair<void*, size_t> CompileShaderSource(const char* path);
}



namespace juye::driver{
VkResult CreateVkBuffer(VkDevice device, VkBuffer* pBuf, size_t bytes, const VkBufferUsageFlags usage);
void DestoryVkBuffers(VkDevice device, VkBuffer* pBufs, uint32_t count, VkAllocationCallbacks* pAllocator);

}

