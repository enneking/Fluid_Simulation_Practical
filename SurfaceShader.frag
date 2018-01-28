#version 330 core

uniform vec3 view_pos;
in vec3 pos;
in vec3 normal;
out vec4 Color;

void main()
{
	vec4 white = vec4(1.0f, 1.0f, 1.0f, 0.2f);

	vec3 light_pos = vec3(20.0f, 20.0f, 20.0f);

	vec4 ambient = vec4(0.0f, 0.0f, 1.0f, 0.3);

	vec3 light_dir = normalize(light_pos - pos);
	float diffuse = dot(light_dir, normal);

	vec3 view_dir = normalize(view_pos - pos);
	vec3 specular_norm = normalize(view_dir + light_dir);

	float specular = 0.00000005 * pow(dot(normal, specular_norm), 10.0f);
	if (specular < 0.9f)
	{
		specular = 0.0f;
	}

	Color = 0.8f * ambient + 0.195 * diffuse * white + specular * white;
}