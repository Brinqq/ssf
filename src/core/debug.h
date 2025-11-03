#pragma once


#include <stdio.h>
#include <cstdlib>

inline void _ssf_runtime_error(const char* file, int line){
  printf("ssf has expeirenced a runtime error. thrown from %s:%i\n", file, line);
  std::abort();
};

#define ssf_runtime_error() _ssf_runtime_error(__FILE__, __LINE__);
