#pragma once

#include "vulkan/vulkan.h"
#include "vk_debug.h"

enum QueueBitTypes{
  QueueBitNone = 0x0,
  QueueBitGraphic = 0x2,
  QueueBitTransfer = 0x4,
  QueueBitCompute = 0x8,
  QueueBitPresent = 0x16,
  QueueBitSparse = 0x32
};


enum ShaderStageType{
  ShaderStageNone = - 1,
  ShaderStageVertex,
  ShaderStagePixel,
  ShaderStageCompute,
};

struct QueueFamily{
  uint8_t index;
  uint8_t maxQueues;
  QueueBitTypes bits;
};

namespace juye::driver{

  enum BuiltinUniformType{
    BuiltinUniformCamera,
    BuiltinUniformCount
  };

  struct DrawFrustum{
    float view[4]; //should this be in here? probably not, who knows.
    float projection[4];
  };

}

