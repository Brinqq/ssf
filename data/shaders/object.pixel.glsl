#version 450

layout(location = 0) in vec2 uv;
layout(location = 1) in vec3 normal;

layout(binding = 0) uniform sampler2D tex;
layout(location = 0) out vec4 col; //actual pixel color we present


struct Directional_light{
  vec3 color;
  vec3 direction;
};

Directional_light test_light = {
  vec3(1.0f, 0.95f, 0.8f),
  vec3(0.7f, -0.7f, -1.0f),
};

// vec4 calculate_light(vec3 n){
//   vec4 ret;
//   float l = max(dot(normal, -test_light.pos), 0.0f);
//   ret = vec4(test_light.color * l, 1.0f);
//   return ret;
// }

vec4 apply_ambient_light(vec4 i){
  vec4 ambient_light = vec4(0.2f, 0.3f, 0.4f, 1.0f);
  return i * ambient_light;
}

vec4 apply_directional_light(vec3 n, vec4 i){
  float d = max(dot(n, -test_light.direction), 0.0f);
  vec4 p = vec4(test_light.color * d, 1.0f);
  return i * p;
}

void main(){
  vec4 albedo = texture(tex, uv);
  vec4 diffuse = apply_directional_light(normal, albedo);
  vec4 ambient = apply_ambient_light(albedo);
  col = diffuse + ambient;

  // vec4 ambient_light = vec4(0.1, 0.0, 0.1, 1.0f);
  // col =  local_texture * calculate_light(normal);
  // col = col + ambient_light;
}
