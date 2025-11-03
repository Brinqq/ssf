#include "ssf/ssf.h"
#include <stdio.h>

#include "gpu/VK/vk_core.h"
#include "device.h"

#include "gfx/gfx.h"

#include <cassert>

namespace ssf{

int Init(){
  VK vk{};
  Device device{};

  vk.Init();
  CreateGraphicSystem(device, vk);
  vk.TestTriangle();

  while(device.IsGraphicWindowRunning()){
    device.Tick();
    vk.Draw();
  };


  return 0;
}

int Init(const SystemType sys){
  if(Init()){return 1;}
  //create systems ...
  return 0;
}


int SystemsCreate(const SystemType sys){
  return 0;
}

}//namespace ssf
