#version 330 core

layout(location = 0) in vec3 VertPos;

uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;

void main(){

	gl_Position = uProjectionMatrix * uViewMatrix * vec4(VertPos,1.0f);

}