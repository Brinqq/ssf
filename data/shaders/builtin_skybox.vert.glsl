#version 450

layout(location = 0)out vec3 texcoord;

layout(set = 1, binding = 0) uniform fustrum{
  mat4 view;
  mat4 projection;
};

vec2 vertices[3] = vec2[3](
  vec2(-1.0, -1.0),
  vec2( 3.0, -1.0),
  vec2(-1.0,  3.0)
);

void main(){
    vec2 pos = vertices[gl_VertexIndex];
    gl_Position = vec4(pos, 0.99, 1.0);


    mat4 invProj = inverse(projection);
    mat4 invView = inverse(mat4(mat3(view)));
    vec4 unprojected = invProj * vec4(pos, 0.0, 1.0);
    texcoord = unprojected.xyz / unprojected.w;
    texcoord *= -1;

}
