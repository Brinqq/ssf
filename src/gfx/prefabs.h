#pragma once

#include <stdint.h>
#include <stddef.h>

namespace ssf{
  namespace prefabs{


  template<typename _IndexType>
  struct StandardCube{
    static constexpr float vertices[24] = {
        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f
    };

    static constexpr uint16_t indices[36] = {
        0, 1, 2,  2, 3, 0,
        5, 4, 7,  7, 6, 5,
        3, 2, 6,  6, 7, 3,
        4, 5, 1,  1, 0, 4,
        1, 5, 6,  6, 2, 1,
        4, 0, 3,  3, 7, 4
    };

    static constexpr uint32_t nVertices = 8;
    static constexpr uint32_t Vertexstride = sizeof(float) * 3;
    static constexpr size_t VertexBytes = Vertexstride * nVertices;

    static constexpr uint32_t nIndices = 36;
    static constexpr uint32_t IndiceStride = sizeof(_IndexType);
    static constexpr size_t IndiceBytes = IndiceStride * nIndices;
};



  };
};
