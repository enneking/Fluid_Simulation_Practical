#version 330 core

in vec2 FragmentPosition;
in vec4 Pos;

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


	// normal of sphere
	vec3 normal = normalize(vec3(FragmentPosNorm, Depth));

	vec3 LightPosition = vec3(2.0f, 2.0f, 2.0f);

	vec3 lightDir = normalize(LightPosition - Pos.xyz);
	vec3 viewDir = normalize(uViewPos - Pos.xyz);

	//compute the diffuse color
	vec3 diffCol = vec3(0.1f) * dot(normal, normalize(lightDir));

	//compute the specular color
	vec3 h = normalize(lightDir + viewDir);
	vec3 specCol = vec3(0.3f) * pow(dot(normal, h), 2.0f);

	//float clampedIntensityCoeff = clamp(dot(LightPosition, normal), 0.0, 1.0);

	//// ambient + diffuse
	//float lightingIntensity = 0.3 + 0.7 * clampedIntensityCoeff;
	//finalSphereColor *= lightingIntensity;

	//// specular lighting
	//lightingIntensity = pow(clampedIntensityCoeff, 60.0f);
	//finalSphereColor *= lightingIntensity;

  color = vec4(uColor * vec3(0.8) + specCol + diffCol, 1.0f);
}