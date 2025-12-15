#version 450

layout(location = 0) in vec2 texcoord;

layout(location = 0) out vec4 pixel;

layout(binding = 0) uniform sampler2D tex;

vec4 skybox_color = vec4(0.4f, 0.5f, 0.6f, 1.0f);

void main(){
  pixel = skybox_color;
}
