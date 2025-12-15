#version 450

layout(location = 0)out vec3 texcoord;

vec2 vertices[3] = vec2[3](
  vec2(-1.0, -1.0),
  vec2( 3.0, -1.0),
  vec2(-1.0,  3.0)
);

void main(){
    vec2 pos = vertices[gl_VertexIndex];
    gl_Position = vec4(pos, 0.99, 1.0);
    
    texcoord = vec3(pos.x, -pos.y + -0.6f, -1.0);
}
