#include "file.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

namespace ssf::core{

ImageData LoadImage(const char* filepath){
  int w, h, c;
  stbi_uc* pixels = stbi_load(filepath, &w, &h, &c, STBI_rgb_alpha);
  size_t bytes = w * h * 4;
  assert(pixels != 0);
  return ImageData{(void*)pixels, bytes, w, h};
}


void UnloadImage(ImageData& image){
  stbi_image_free(image.data);
}

}
