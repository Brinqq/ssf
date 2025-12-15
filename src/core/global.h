#pragma once
#include <stddef.h>
#include <stdint.h>
#include <cassert>
#include <stdio.h>
#include <cstdlib>

inline void _juye_runtime_error(const char* file, int line){
  printf("ssf has expeirenced a runtime error. thrown from %s:%i\n", file, line);
  std::abort();
};

#define juye_runtime_error() _juye_runtime_error(__FILE__, __LINE__);
#define juye_runtime_errorf() _juye_runtime_error(__FILE__, __LINE__);

#if !defined(NDEBUG)

inline void _juye_assert_msg(const char* expr, const char* msg, const char* file, int line){
};


#define juye_assertf(_expr, _msg) if(!_expr){_juye_assert_msg(#_expr, #_msg, __FILE__, __LINE__)}
#else
#define juye_assertf()
#endif

