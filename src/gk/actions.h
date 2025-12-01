#pragma once
#include "keyboard.h"

#include <stdint.h>

namespace ssf{
  typedef uint16_t ActionHandle;

  ActionHandle CreateAction(); 
  void DestroyAction(ActionHandle);
  int MapAction(ActionHandle handle, KeyCode code);
  void UnmapAction(ActionHandle, KeyCode code);
  int CheckAction(ActionHandle handle);
}
