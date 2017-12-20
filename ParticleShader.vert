#version 330 core 

layout(location = 0) in vec3 VertexPosition;

uniform mat4 ViewProjection;


void main(){
  gl_Position = ViewProjection * vec4(VertexPosition, 1.0f);
}