#version 450

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec3 normal;

layout(push_constant) uniform matrix{
  mat4 model;
  mat4 projection;
};

void main(){
  gl_Position = projection * model * vec4(vertex, 1.0f);
}
