#version 330 core

uniform vec3 view_pos;
in vec3 pos;
in vec3 normal;
out vec4 Color;

void main()
{
	vec3 white = vec3(1.0f, 1.0f, 1.0f);

	vec3 light_pos = vec3(20.0f, 20.0f, 20.0f);

	vec3 ambient = vec3(0.0f, 0.0f, 1.0f);

	vec3 light_dir = normalize(light_pos - pos);
	float diffuse = dot(light_dir, normal);

	vec3 view_dir = normalize(view_pos - pos);
	vec3 specular_norm = normalize(view_dir + light_dir);

	float specular = 0.01 * pow(dot(normal, specular_norm), 10.0);

	Color.xyz = vec3(0.4f * ambient + 0.1 * diffuse * white + specular * white) + 0.5 * white;
}