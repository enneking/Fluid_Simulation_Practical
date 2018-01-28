#version 330 core

layout(location = 0) in vec3 VertPos;
layout(location = 1) in vec3 VertNormal;


uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;

out vec3 pos;
out vec3 normal;

void main(){

	normal = -1.0f * VertNormal;

	pos = VertPos;


	gl_Position = uProjectionMatrix * uViewMatrix * vec4(VertPos,1.0f);

}