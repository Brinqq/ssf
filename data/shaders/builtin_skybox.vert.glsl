#version 450

layout(location = 0)out vec2 texcoord;

vec2 vertices[3] = vec2[3](
  vec2(-1.0, -1.0),
  vec2( 3.0, -1.0),
  vec2(-1.0,  3.0)
);

void main(){
    gl_Position = vec4(vertices[gl_VertexIndex], 0.99f, 1.0);
    texcoord = vertices[gl_VertexIndex] * 0.5 + 0.5;
}
