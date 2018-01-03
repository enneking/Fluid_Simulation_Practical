#version 330 core 

layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec2 BillboardVertex;

uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;

uniform vec3 uUpVector;
uniform vec3 uRightVector;

out vec4 Pos;

out vec2 FragmentPosition;

void main(){
	FragmentPosition = BillboardVertex.xy;

	Pos = uViewMatrix * vec4(VertexPosition + (BillboardVertex.x * uRightVector) + (BillboardVertex.y * uUpVector), 1.0f);

  gl_Position = uProjectionMatrix * Pos;
}