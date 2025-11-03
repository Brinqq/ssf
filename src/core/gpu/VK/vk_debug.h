#pragma once

#include "vk_defines.h"

#include <stdio.h>
#include <cstdlib>


inline void VKResultError(int code, const char* file, int line){
  printf("VkResult error code: %i - %s:%i\n", code, file, line);
  std::abort();
}

static int g_error;

#define vkcall(_expr) g_error =  _expr; if(g_error != VK_SUCCESS){ VKResultError(g_error, __FILE__, __LINE__);}
