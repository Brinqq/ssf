#pragma once
#include "core/global.h"

#include <bcl/memory/linearallocator.h>

namespace juye{

enum AssetType{
  model
};//enum AssetType

struct Asset{
  size_t id;
  void* pVertices;
  void* pIndices;
  size_t numVertices;
  size_t numIndices;
};//struct AssetType

class AssetRegistry{
private:
  static constexpr size_t kMaxAssetMemory = bk::AlignP2(3 * GiB, 16);

  //HACK: for now we are just doing not allowing removing indivual assets.
  bk::LinearAllocator<16> mAllocator;
  void* mMemory;
public:
  Asset LoadModelFromGLTF(const char* pFile);
  Asset GetCachedAsset(const char* filename);

  AssetRegistry();
  ~AssetRegistry();

  AssetRegistry(const AssetRegistry& rhs) = delete;
  AssetRegistry& operator=(const AssetRegistry& rhs) = delete;
  AssetRegistry(AssetRegistry&& rhs) = delete;
  AssetRegistry& operator=(AssetRegistry&& rhs) = delete;

};// class AssetRegistry

}// namespace juye
