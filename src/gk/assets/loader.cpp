#include "loader.h"

#include "core/global.h"

#include <assimp/cimport.h>
#include <assimp/mesh.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>

using namespace juye;


Mesh LoadGLTF(const char* pFile){
  const aiScene* scene = aiImportFile(pFile, 0);
  if(scene == nullptr){
    printf("%s\n", aiGetErrorString());
    juye_runtime_error();
  }
   aiMesh* k = *scene->mMeshes;
   aiTexture* t = *scene->mTextures;
   aiMaterial* m = *scene->mMaterials;

   return Mesh{};
}

