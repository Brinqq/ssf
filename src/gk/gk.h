#pragma once

#include "core/device.h"

//public ssf
#include "keyboard.h"
#include "actions.h"

class VK;

class GK{
private:
  VK* driver;
  Device device;
public:
  //for now we just take in the core system
  int Init(VK& driver);
  void Destroy();
  void Tick();

};

void CreateGraphicSystem(Device& device, VK& vulkan);
