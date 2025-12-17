#include "juye/gk/prefabs.h"

using namespace juye;

static float kPlaneVertices[32] = {
  -0.5f, 0.0f, -0.5f,   0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
   0.5f, 0.0f, -0.5f,   1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
   0.5f, 0.0f,  0.5f,   1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
  -0.5f, 0.0f,  0.5f,   0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
};

static uint16_t kPlaneIndices[6] = {
  0, 1, 2, 2, 3, 0
};


static float kCubeVertices[192] = {  // 24 vertices × 5 floats
  -0.5f, -0.5f,  0.5f,   0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
   0.5f, -0.5f,  0.5f,   1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
   0.5f,  0.5f,  0.5f,   1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
  -0.5f,  0.5f,  0.5f,   0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
  
   0.5f, -0.5f, -0.5f,   0.0f, 0.0f, 0.0f, 0.0f, -1.0f,
  -0.5f, -0.5f, -0.5f,   1.0f, 0.0f, 0.0f, 0.0f, -1.0f,
  -0.5f,  0.5f, -0.5f,   1.0f, 1.0f, 0.0f, 0.0f, -1.0f,
   0.5f,  0.5f, -0.5f,   0.0f, 1.0f, 0.0f, 0.0f, -1.0f,
  
  -0.5f,  0.5f,  0.5f,   0.0f, 0.0f, 0.0f, -1.0f, 0.0f,
   0.5f,  0.5f,  0.5f,   1.0f, 0.0f, 0.0f, -1.0f, 0.0f,
   0.5f,  0.5f, -0.5f,   1.0f, 1.0f, 0.0f, -1.0f, 0.0f,
  -0.5f,  0.5f, -0.5f,   0.0f, 1.0f, 0.0f, -1.0f, 0.0f,
  
  -0.5f, -0.5f, -0.5f,   0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
   0.5f, -0.5f, -0.5f,   1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
   0.5f, -0.5f,  0.5f,   1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
  -0.5f, -0.5f,  0.5f,   0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
  
   0.5f, -0.5f,  0.5f,   0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
   0.5f, -0.5f, -0.5f,   1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
   0.5f,  0.5f, -0.5f,   1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
   0.5f,  0.5f,  0.5f,   0.0f, 1.0f, 1.0f, 0.0f, 0.0f,
  
  -0.5f, -0.5f, -0.5f,   0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
  -0.5f, -0.5f,  0.5f,   1.0f, 0.0f, -1.0f, 0.0f, 0.0f,
  -0.5f,  0.5f,  0.5f,   1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
  -0.5f,  0.5f, -0.5f,   0.0f, 1.0f, -1.0f, 0.0f, 0.0f,
};


static  uint16_t kCubeIndices[36] = {
    0,  1,  2,   2,  3,  0,   
    4,  5,  6,   6,  7,  4,   
    8,  9, 10,  10, 11,  8,   
   12, 13, 14,  14, 15, 12,   
   16, 17, 18,  18, 19, 16,   
   20, 21, 22,  22, 23, 20    
};


Prefab Prefab::Builder::Build() noexcept{
  Prefab ret;
  ret.pVertices = this->pVertices;
  ret.pIndices = this->pIndices;
  ret.indices = this->indices;
  ret.vertices = this->vertices;
  ret.stride = this->stride;
  return ret;
};

Prefab::Builder& Prefab::Builder::SetMesh(const Prefab::BuiltinMeshType type) noexcept{
  switch (type){

    case PrefabMeshCube:
    this->pIndices = kCubeIndices;
    this->pVertices = kCubeVertices;
    this->indices = 36;
    this->vertices = 24;
    this->stride = sizeof(float) * 8;
    return *this;

    case PrefabMeshPlane:
    this->indices = 6;
    this->pIndices = kPlaneIndices;
    this->vertices = 4;
    this->stride = sizeof(float) * 8;
    this->pVertices = kPlaneVertices;
    return *this;

    }

  
  return *this;
}


Prefab::Builder& Prefab::Builder::SetTexture(const BuiltinTextureType type) noexcept{}

