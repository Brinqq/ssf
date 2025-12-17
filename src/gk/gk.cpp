#include "gk.h"
#include "juye/gk/prefabs.h"
#include "core/global.h"

#include "core/drivers/device.h"
#include "core/fsystem/file.h"
#include "core/drivers/VK/vk_core.h"
#include "core/configuration/build_generation.h"
#include "core/debug.h"

#include <stdio.h>
#include <cassert>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>


// void CreateGraphicSystem(Device& device, VK& vulkan){
//   device.CreateGraphicWindow(1920, 1080, "sim");
//   vulkan.CreateGraphicsState(device);
// }

using namespace juye;

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

Camera cam{};
bool updateCam = false;
float speed = 0.04;

juye::ActionHandle forward;
juye::ActionHandle left;
juye::ActionHandle right;
juye::ActionHandle back;
juye::ActionHandle up;
juye::ActionHandle down;
juye::ActionHandle anExit;

void DevInitInput(){
  forward = juye::CreateAction();
  left = juye::CreateAction();
  right = juye::CreateAction();
  back = juye::CreateAction();
  up = juye::CreateAction();
  down = juye::CreateAction();
  anExit = juye::CreateAction();

  juye::MapAction(forward, juye::KeyCodeW);
  juye::MapAction(right, juye::KeyCodeD);
  juye::MapAction(left, juye::KeyCodeA);
  juye::MapAction(back, juye::KeyCodeS);
  juye::MapAction(up, juye::KeyCodeSHIFT);
  juye::MapAction(down, juye::KeyCodeSPACE);
  juye::MapAction(anExit, juye::KeyCodeESC);

};

void DevUpdateView(){
  simpleCube.push[1] = cam.view;
  plane.push[1] = cam.view;
}

void DevUpdateInput(){
  if(juye::CheckAction(forward)){
   cam.view = glm::translate(cam.view, glm::vec3(0.0f, 0.0f, -1.0f * speed));
  }

  if(juye::CheckAction(back)){
   cam.view = glm::translate(cam.view, glm::vec3(0.0f, 0.0f, 1.0f * speed));
  }

  if(juye::CheckAction(left)){
   cam.view = glm::translate(cam.view, glm::vec3(-1.0f * speed, 0.0f, 0.0f));
  }

  if(juye::CheckAction(right)){
   cam.view = glm::translate(cam.view, glm::vec3(1.0f * speed, 0.0f, 0.0f));
  }

  if(juye::CheckAction(up)){
   cam.view = glm::translate(cam.view, glm::vec3(0.0f , -1.0f * speed, 0.0f));
  }

  if(juye::CheckAction(down)){
   cam.view = glm::translate(cam.view, glm::vec3(0.0f, 1.0f * speed, 0.0f));
  }

  if(juye::CheckAction(anExit)){
    gApplicationClose = true;
  }


  DevUpdateView();
  
}



ResourceHandle skybox;

juye::ImageData skyboxData[6];

CubeMapWriteDescription CubeMapDataGenerate(){
  skyboxData[5]  = juye::LoadImage("/Users/brinq/.dev/projects/solar-sim/juye/data/textures/px.png");
  skyboxData[4]  = juye::LoadImage("/Users/brinq/.dev/projects/solar-sim/juye/data/textures/nx.png");
  skyboxData[1]  = juye::LoadImage("/Users/brinq/.dev/projects/solar-sim/juye/data/textures/py.png");
  skyboxData[0]  = juye::LoadImage("/Users/brinq/.dev/projects/solar-sim/juye/data/textures/ny.png");
  skyboxData[2]  = juye::LoadImage("/Users/brinq/.dev/projects/solar-sim/juye/data/textures/pz.png");
  skyboxData[3]  = juye::LoadImage("/Users/brinq/.dev/projects/solar-sim/juye/data/textures/nz.png");

  CubeMapWriteDescription ret{};
  for(int i = 0; i < 7; ++i){
    if(skyboxData[i].data == nullptr){juye_runtime_error();}
    ret.data[i] = skyboxData[i].data;
    ret.bytes[i] = skyboxData[i].bytes;
  }

  return ret;
}

void CubeMapDataCleanup(){
  for(int i = 0; i < 6; ++i){
    juye::UnloadImage(skyboxData[i]);
  }
}

int GK::Init(VK& vulkan){
  driver = &vulkan;
  device.CreateGraphicWindow(1920, 1080, "sim");
  driver->CreateGraphicsState(device);
  DevInitInput();

  cam.view = glm::lookAt(glm::vec3(0.0f, -4.0f, -4.0f), glm::vec3(0.0f, 1.0f, 4.0f), glm::vec3(0.0f, 1.0f, 0.0f));
  cam.projection = glm::perspectiveFov(glm::radians(60.0f), static_cast<float>(device.windowW), static_cast<float>(device.windowH), 0.1f, 1000.0f);

  simpleCube.push[0] = glm::mat4(1);
  simpleCube.push[1] = cam.view;
  simpleCube.push[2] = cam.projection;

  plane.push[0] = glm::mat4(1);
  plane.push[1] = cam.view;
  plane.push[2] = cam.projection;

  simpleCube.push[0] = glm::translate(simpleCube.push[0], glm::vec3(0.0f, -0.7f, 2.0f));
  plane.push[0] = glm::scale(plane.push[0], glm::vec3(20.0f, 0.0f, 20.0f));


  // juye::prefabs::TexturedCube<uint16_t> cube;
  // juye::prefabs::TexturedPlane<uint16_t> planePrefab;



  Prefab cube = Prefab::Builder()
               .SetMesh(Prefab::PrefabMeshCube)
               .Build();

  Prefab planePrefab = Prefab::Builder()
               .SetMesh(Prefab::PrefabMeshCube)
               .Build();

  std::string texpath(_SSF_GENERATED_TEXTURE_FOLDER);;
  std::string texpath2(_SSF_GENERATED_TEXTURE_FOLDER);;
  juye::ImageData image  = juye::LoadImage(texpath.append("def.png").c_str());
  juye::ImageData pimage  = juye::LoadImage(texpath2.append("orange.png").c_str());

  GeometryData dat;
  GeometryData geometryPlane;

  dat.pVertex = (void*)cube.pVertices;
  dat.pIndices = (void*)cube.pIndices;
  dat.indicesBytes = cube.indices * sizeof(uint16_t);
  dat.vertexBytes = cube.vertices * cube.stride;
  dat.numIndices = cube.indices;
  dat.texture = image.data;
  dat.textureWidth = image.width;
  dat.textureHeight = image.height;

  geometryPlane.pVertex = (void*)planePrefab.pVertices;
  geometryPlane.pIndices = (void*)planePrefab.pIndices;
  geometryPlane.indicesBytes = planePrefab.indices * sizeof(uint16_t);
  geometryPlane.vertexBytes = planePrefab.vertices * planePrefab.stride;
  geometryPlane.numIndices = planePrefab.indices;
  geometryPlane.texture = pimage.data;
  geometryPlane.textureWidth = pimage.width;
  geometryPlane.textureHeight = pimage.height;

  simpleCube.handle = driver->CreateGeometry(dat);
  plane.handle = driver->CreateGeometry(geometryPlane);

  driver->MapGeometryPassPushBuf(simpleCube.handle, simpleCube.push);
  driver->MapGeometryPassPushBuf(plane.handle, plane.push);

  juye::UnloadImage(image);
  juye::UnloadImage(pimage);

  CubeMapWriteDescription x = CubeMapDataGenerate();
  skybox =  driver->CreateCubeMap(skyboxData[0].width);
  driver->WriteCubeMap(skybox ,x);
  driver->SetSkyBox(skybox);
  CubeMapDataCleanup();

  gApplicationClose = false;
  return 0;
}

void GK::Tick(){
    if(!device.IsGraphicWindowRunning()){
      gApplicationClose = true;
      return;
    }

    driver->WriteFrustum(reinterpret_cast<float*>(&cam));

    device.Tick();
    
    driver->AddToDrawList(simpleCube.handle);
    driver->AddToDrawList(plane.handle);
    driver->Draw();

    DevUpdateInput();

};

void GK::Destroy(){
  driver->DestroyCubeMap(skybox);
  // driver->Destroy();
  device.CloseGraphicWindow();
}
