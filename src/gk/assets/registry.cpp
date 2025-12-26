#include "registry.h"

#include "assimp/material.h"
#include "core/global.h"

#include "bcl/memory/bkmemory.h"

#include <assimp/cimport.h>
#include <assimp/mesh.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using namespace juye;

Asset AssetRegistry::LoadModelFromGLTF(const char* pFile){
  Asset ret{};
  const aiScene* scene = aiImportFile(pFile, 
  aiProcess_Triangulate);
  if(scene == nullptr){
    printf("%s\n", aiGetErrorString());
    juye_runtime_error();
  }
   struct vertex{
    aiVector3D pos;
    aiVector2D uv;
    aiVector3D normal;
   };

  struct triangle{
    uint16_t i0;
    uint16_t i1;
    uint16_t i2;
  };

   size_t vertexBytes = 0;
   size_t indiceBytes = 0;

  //size pass
   for(int i = 0; i < scene->mNumMeshes; ++i){
    aiMesh* mesh = scene->mMeshes[i];
    indiceBytes += sizeof(triangle) * mesh->mNumFaces;
    vertexBytes += sizeof(vertex) * mesh->mNumVertices;
   }

  //allocate memory
  ret.pVertices = mAllocator.TryAlloc(vertexBytes);
  ret.pIndices = mAllocator.TryAlloc(indiceBytes);

  if(ret.pIndices == nullptr|| ret.pVertices == nullptr){
    juye_runtime_error();
  }

  //fill pass
  vertex* vert = static_cast<vertex*>(ret.pVertices);
  triangle* ind = static_cast<triangle*>(ret.pIndices);
  size_t voff = 0;


  for(int x = 0; x < scene->mNumMeshes; ++x){
    aiMesh* mesh = scene->mMeshes[x];
    ret.numVertices += mesh->mNumVertices;
    ret.numIndices += mesh->mNumFaces * 3;

    for(int m = 0; m < mesh->mNumVertices; ++m){
      vertex v{};
      v.pos = mesh->mVertices[m];

      if(mesh->mTextureCoords[0]!= nullptr){
        v.uv.x = mesh->mTextureCoords[0][m].x;
        v.uv.y = mesh->mTextureCoords[0][m].y;
      }else{
        v.uv.x = 0.0f;
        v.uv.y = 0.0f;
      }
      
      v.normal = mesh->mNormals ? mesh->mNormals[m] : aiVector3D{0, 1, 0};

      *vert = v;
      vert++;
    }

    for(int i = 0; i < mesh->mNumFaces; ++i){
      triangle t;
      aiFace face = mesh->mFaces[i];
       t.i0 = face.mIndices[0] + voff;
       t.i1 = face.mIndices[1] + voff;
       t.i2 = face.mIndices[2] + voff;

       *ind = t;
       ind++;
    }
    voff += mesh->mNumVertices;
  }

   return ret;
}

  Asset AssetRegistry::GetCachedAsset(const char* filename){
    return Asset{};
  }


AssetRegistry::AssetRegistry(){
  mMemory = malloc(kMaxAssetMemory);
  mAllocator.Set(mMemory, kMaxAssetMemory);
}

AssetRegistry::~AssetRegistry(){
  free(mMemory);
}
