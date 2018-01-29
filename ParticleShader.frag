#version 330 core

in vec2 FragmentPosition;
in vec4 Pos;

uniform mat4 uViewMatrix;
uniform float uParticleRadius;
uniform float uViewPos;

out vec4 color;

uniform vec3 uColor;

void main(){

	vec2 FragmentPosNorm = FragmentPosition / uParticleRadius;

	float distance = length(FragmentPosNorm);

	if (distance > 1.0f)
	{
		discard;
	}

	float Depth = sqrt(1.0 - distance * distance);

	mat4 inv_view = inverse(uViewMatrix);
	// normal of sphere
	vec3 normal = (inv_view * vec4(FragmentPosNorm, Depth, 0.0f)).xyz;
	vec3 surface_pos = (Pos + inv_view * vec4(0.0f, 0.0f, Depth * uParticleRadius, 0.0f)).xyz;

	vec3 LightPosition = vec3(20.0f, 20.0f, 20.0f);

	vec3 lightDir = normalize(LightPosition - surface_pos);
	vec3 viewDir = normalize(uViewPos - surface_pos);

	//compute the diffuse color
	vec3 diffCol = vec3(0.1f) * dot(normal, lightDir);

	//compute the specular color
	vec3 h = normalize(lightDir + viewDir);
	vec3 specCol = vec3(0.3f) * pow(dot(normal, h), 3.0f);


  color = vec4(uColor * vec3(0.8) + specCol + diffCol, 1.0f);
}