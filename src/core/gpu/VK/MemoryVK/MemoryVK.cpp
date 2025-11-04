#include "MemoryVK.h"

//for now we just grab a whole page each allocation.
VkResult MemoryVK::Allocate(VkDevice device, VkDeviceMemory* handle, VkDeviceSize bytes, uint32_t type){
  const VkMemoryAllocateInfo mem{
   .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
   .pNext = 0,
   .allocationSize = bytes,
   .memoryTypeIndex = type
  };

  return vkAllocateMemory(device, &mem, nullptr, handle);
}
