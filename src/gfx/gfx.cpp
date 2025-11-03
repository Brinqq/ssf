#include "gfx.h"

#include <stdio.h>
#include <cassert>

#include "core/device.h"
#include "core/gpu/VK/vk_core.h"

void CreateGraphicSystem(Device& device, VK& vulkan){
  device.CreateGraphicWindow(1920, 1080, "sim");
  vulkan.CreateGraphicsState(device);
}
