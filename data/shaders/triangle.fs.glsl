#version 450

layout(location = 0) in vec3 in_color;
layout(location = 1) in vec3 in_position;
layout(location = 2) in vec3 in_normal;

layout(location = 0) out vec4 out_color;


void main()
{
	vec3 light_position = vec3(1.0f, 8.0f, 3.0f);
	vec3 light_color = vec3(1.0f);

	vec3 ambient = 0.2f * light_color;

	vec3 norm = normalize(in_normal);
	vec3 light_direction = normalize(light_position - in_position);
	float diff_coef = max(dot(norm, light_direction), 0.0f);
	vec3 diffuse = light_color * diff_coef;
	
	vec3 color = (ambient + diffuse) * in_color;
	out_color = vec4(color, 1.0f);
}
