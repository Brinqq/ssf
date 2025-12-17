#pragma once
#include "vkdefines.h"

namespace juye::driver{

class DescriptorSetLayoutBuilder{
private:
  static constexpr int kMaxBindings = 5;
  VkDescriptorSetLayoutBinding bindings[kMaxBindings];
  uint32_t count;
public:
  DescriptorSetLayoutBuilder& AddBinding(uint32_t idx, uint32_t batch, const VkDescriptorType type, const VkShaderStageFlags stages) noexcept;
  VkDescriptorSetLayout Build(VkDevice device, VkAllocationCallbacks* allocator) noexcept;

};// DescriptorSetLayoutBuilder
  

  VkResult AllocateVkDescriptorSets(VkDevice device, const VkDescriptorPool& pool, const VkDescriptorSetLayout* pLayouts, 
           uint32_t count, VkDescriptorSet* pSets);
  
}// namespace juye::driver

