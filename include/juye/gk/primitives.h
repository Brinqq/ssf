#pragma once

namespace juye{

struct Color3{
  float r;
  float g;
  float b;
};

struct Vector3f{
  float x;
  float y;
  float z;
};


template<typename _Type>
struct vec3{
  union {
    struct {_Type r, g, b;};
    struct{_Type x, y, z;};
  };
};

}
