#include "ssf/ssf.h"
#include <stdio.h>

#include "gpu/VK/vk_core.h"
#include "device.h"

#include "gk/gk.h"

#include <cassert>

bool gApplicationClose = true;

namespace ssf{

int Init(){
  //core systems
  VK driver{};
  driver.Init();

  //subsystems
  GK graphics;
  graphics.Init(driver);


  while(!gApplicationClose){
    graphics.Tick();
  };


  graphics.Destroy();
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
