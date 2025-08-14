#version 460

layout(set = 1, binding = 0) uniform GlobalData
{
	vec3 color_shift;
};

layout(location = 0) in vec4 color;

layout(location = 0) out vec4 frag_color;

void main()
{
	frag_color = vec4(color.rgb + color_shift*0.3f, color.a);
}
