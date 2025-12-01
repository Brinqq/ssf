#include "gk.h"
#include "gk/prefabs.h"

#include "core/device.h"
#include "core/fsystem/file.h"
#include "core/gpu/VK/vk_core.h"
#include "core/configuration/build_generation.h"

#include <stdio.h>
#include <cassert>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>


// void CreateGraphicSystem(Device& device, VK& vulkan){
//   device.CreateGraphicWindow(1920, 1080, "sim");
//   vulkan.CreateGraphicsState(device);
// }

extern bool gApplicationClose;

struct Camera{
  glm::mat4 view;
  glm::mat4 projection;
};

struct Mesh{
  VK::GeoHandle handle;
  glm::mat4 transform;
  glm::mat4 push[3];
};

Mesh simpleCube;
Mesh plane;

Camera cam;
bool updateCam = false;
float speed = 0.04;

ssf::ActionHandle forward;
ssf::ActionHandle left;
ssf::ActionHandle right;
ssf::ActionHandle back;
ssf::ActionHandle up;
ssf::ActionHandle down;

void DevInitInput(){
  forward = ssf::CreateAction();
  left = ssf::CreateAction();
  right = ssf::CreateAction();
  back = ssf::CreateAction();
  up = ssf::CreateAction();
  down = ssf::CreateAction();

  ssf::MapAction(forward, ssf::KeyCodeW);
  ssf::MapAction(right, ssf::KeyCodeD);
  ssf::MapAction(left, ssf::KeyCodeA);
  ssf::MapAction(back, ssf::KeyCodeS);
  ssf::MapAction(up, ssf::KeyCodeSHIFT);
  ssf::MapAction(down, ssf::KeyCodeSPACE);

};

void DevUpdateView(){
  simpleCube.push[1] = cam.view;
  plane.push[1] = cam.view;
}

void DevUpdateInput(){
  if(ssf::CheckAction(forward)){
   cam.view = glm::translate(cam.view, glm::vec3(0.0f, 0.0f, -1.0f * speed));
  }

  if(ssf::CheckAction(back)){
   cam.view = glm::translate(cam.view, glm::vec3(0.0f, 0.0f, 1.0f * speed));
  }

  if(ssf::CheckAction(left)){
   cam.view = glm::translate(cam.view, glm::vec3(-1.0f * speed, 0.0f, 0.0f));
  }

  if(ssf::CheckAction(right)){
   cam.view = glm::translate(cam.view, glm::vec3(1.0f * speed, 0.0f, 0.0f));
  }

  if(ssf::CheckAction(up)){
   cam.view = glm::translate(cam.view, glm::vec3(0.0f , -1.0f * speed, 0.0f));
  }

  if(ssf::CheckAction(down)){
   cam.view = glm::translate(cam.view, glm::vec3(0.0f, 1.0f * speed, 0.0f));
  }


  DevUpdateView();
  
}



int GK::Init(VK& vulkan){
  driver = &vulkan;
  device.CreateGraphicWindow(1920, 1080, "sim");
  driver->CreateGraphicsState(device);
  DevInitInput();

  cam.view = glm::lookAt(glm::vec3(0.0f, -4.0f, -4.0f), glm::vec3(0.0f, 1.0f, 4.0f), glm::vec3(0.0f, 1.0f, 0.0f));
  cam.projection = glm::perspectiveFov(glm::radians(60.0f), static_cast<float>(device.windowW), static_cast<float>(device.windowH), 0.1f, 100.0f);

  simpleCube.push[0] = glm::mat4(1);
  simpleCube.push[1] = cam.view;
  simpleCube.push[2] = cam.projection;

  plane.push[0] = glm::mat4(1);
  plane.push[1] = cam.view;
  plane.push[2] = cam.projection;

  simpleCube.push[0] = glm::translate(simpleCube.push[0], glm::vec3(0.0f, -0.7f, 2.0f));
  // plane.push[0] = glm::translate(plane.push[0], glm::vec3(0.0f, -0.2f, 0.0f));
  plane.push[0] = glm::scale(plane.push[0], glm::vec3(20.0f, 0.0f, 20.0f));


  ssf::prefabs::TexturedCube<uint16_t> cube;
  ssf::prefabs::TexturedPlane<uint16_t> planePrefab;

  std::string texpath(_SSF_GENERATED_TEXTURE_FOLDER);;
  std::string texpath2(_SSF_GENERATED_TEXTURE_FOLDER);;
  ssf::core::ImageData image  = ssf::core::LoadImage(texpath.append("def.png").c_str());
  ssf::core::ImageData pimage  = ssf::core::LoadImage(texpath2.append("orange.png").c_str());

  GeometryData dat;
  GeometryData geometryPlane;

  dat.pVertex = (void*)cube.vertices;
  dat.pIndices = (void*)cube.indices;
  dat.indicesBytes = cube.IndiceBytes;
  dat.vertexBytes = cube.VertexBytes;
  dat.numIndices = cube.nIndices;
  dat.texture = image.data;
  dat.textureWidth = image.width;
  dat.textureHeight = image.height;

  geometryPlane.pVertex = (void*)planePrefab.vertices;
  geometryPlane.pIndices = (void*)planePrefab.indices;
  geometryPlane.indicesBytes = planePrefab.IndiceBytes;
  geometryPlane.vertexBytes = planePrefab.VertexBytes;
  geometryPlane.numIndices = planePrefab.nIndices;
  geometryPlane.texture = pimage.data;
  geometryPlane.textureWidth = pimage.width;
  geometryPlane.textureHeight = pimage.height;

  simpleCube.handle = driver->CreateGeometry(dat);
  plane.handle = driver->CreateGeometry(geometryPlane);

  driver->MapGeometryPassPushBuf(simpleCube.handle, simpleCube.push);
  driver->MapGeometryPassPushBuf(plane.handle, plane.push);

  ssf::core::UnloadImage(image);
  ssf::core::UnloadImage(pimage);

  gApplicationClose = false;
  return 0;
}

void GK::Tick(){
    if(!device.IsGraphicWindowRunning()){
      gApplicationClose = true;
      return;
    }

    device.Tick();
    driver->AddToDrawList(simpleCube.handle);
    driver->AddToDrawList(plane.handle);
    driver->Draw();

    DevUpdateInput();
};

void GK::Destroy(){
  // driver->Destroy();
  device.CloseGraphicWindow();
}
