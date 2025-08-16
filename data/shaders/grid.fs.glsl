#version 460

layout(set = 1, binding = 0) uniform GlobalData
{
	vec4 grid_color;
};

layout(location = 0) in vec3 world_position;

layout(location = 0) out vec4 frag_color;

float DrawGrid(float cell_size, float line_width)
{
	float cell_halfsize = cell_size*0.5f;
	vec2 xzdd = fwidth(world_position.xz);
	vec2 grid = 1.0f - abs(mod(world_position.xz, cell_size)*2.0f - 1.0f);
	grid = smoothstep(max(vec2(line_width), xzdd) + xzdd*1.5f, max(vec2(line_width), xzdd) - xzdd*1.5f, grid);
	grid *= clamp(line_width / max(vec2(line_width), xzdd), 0.0f, 1.0f);
	float grid_value = mix(grid.x, 1.0f, grid.y);
	return grid_value;
}

void main(void)
{
	// --AlNov: @TODO To understand what is going on there :)
	float grid_value = DrawGrid(1.0f, .02f);
	float subgrid_value = DrawGrid(0.1f, .01f);

	frag_color = grid_color;
	frag_color.a = max(grid_value, subgrid_value*0.8f);
}
