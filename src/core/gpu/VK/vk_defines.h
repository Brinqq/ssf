#pragma once

#include "vulkan/vulkan.h"

enum QueueBitTypes{
  QueueBitNone = 0x0,
  QueueBitGraphic = 0x2,
  QueueBitTransfer = 0x4,
  QueueBitCompute = 0x8,
  QueueBitPresent = 0x16,
  QueueBitSparse = 0x32
};

struct QueueFamily{
  uint8_t index;
  uint8_t maxQueues;
  QueueBitTypes bits;
};
