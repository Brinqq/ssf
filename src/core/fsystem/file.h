#pragma once

#include "core/global.h"

namespace ssf::core{

struct ImageData{
  void* data;
  size_t bytes;
  int height;
  int width;
};

  ImageData LoadImage(const char* filepath);
  void UnloadImage(ImageData& image);
}
