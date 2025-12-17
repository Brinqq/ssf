#include "vkresource.h"

using namespace juye::driver;

DescriptorSetLayoutBuilder& DescriptorSetLayoutBuilder::AddBinding(uint32_t idx, uint32_t batch,
                          const VkDescriptorType type, const VkShaderStageFlags stages) noexcept{

  VkDescriptorSetLayoutBinding binding{};
  binding.binding = idx;
  binding.descriptorCount = batch;
  binding.descriptorType = type;
  binding.pImmutableSamplers = nullptr;
  binding.stageFlags = stages;

  bindings[count] = binding;
  count++;

  return *this;
}

VkDescriptorSetLayout DescriptorSetLayoutBuilder::Build(VkDevice device, VkAllocationCallbacks* allocator) noexcept{
  VkDescriptorSetLayout ret;

  VkDescriptorSetLayoutCreateInfo layout{
   VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
    nullptr,
    0, // configurable maybe?
    count,
    bindings,
  };

  vkcall(vkCreateDescriptorSetLayout(device, &layout, allocator, &ret))
  return ret;
}


  VkResult juye::driver::AllocateVkDescriptorSets(VkDevice device, const VkDescriptorPool& pool, 
            const VkDescriptorSetLayout* pLayouts, uint32_t count, VkDescriptorSet* pSets){

    VkDescriptorSetAllocateInfo info{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, 0, pool, count, pLayouts };
    return vkAllocateDescriptorSets(device, &info, pSets);
  }




