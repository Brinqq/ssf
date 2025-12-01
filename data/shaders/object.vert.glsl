#version 450

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normal;

layout(push_constant) uniform push{
  mat4 model;
  mat4 view;
  mat4 projection;
};

layout(location = 0) out vec2 out_uv;
layout(location = 1) out vec3 out_normal;

void main(){
    gl_Position = projection * view* model * vec4(vertex, 1.0);
    out_uv = uv;
    out_normal = normal;
}
