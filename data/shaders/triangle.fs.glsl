#version 450

layout(location = 0) in vec3 in_color;
layout(location = 1) in vec3 in_position;
layout(location = 2) in vec3 in_normal;

layout(location = 0) out vec4 out_color;


void main()
{
	vec3 light_position = vec3(2.0f, 1.0f, 2.0f);
	vec3 light_color = vec3(1.0f);

	vec3 ambient = 0.4f * light_color;

	vec3 norm = normalize(in_normal);
	// vec3 light_direction = normalize(light_position - in_position);
	vec3 light_direction = normalize(vec3(0.0f, 0.0f, -1.0f));
	float diff_coef = max(dot(-light_direction, norm), 0.0f);
	vec3 diffuse = diff_coef * light_color;

	vec3 color = (ambient + diffuse) * in_color;
	out_color = vec4(color, 1.0f);
}

