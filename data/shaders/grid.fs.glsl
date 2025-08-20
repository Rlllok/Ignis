#version 460

layout(set = 2, binding = 0) uniform GlobalData
{
	vec4 grid_color;
};

layout(location = 0) in vec3 world_position;

layout(location = 0) out vec4 frag_color;

float DrawGrid(float cell_size, float line_width)
{
  line_width = clamp(line_width, 0.0f, 1.0f);
	vec2 xzdd = fwidth(world_position.xz);
  vec2 draw_width = max(vec2(line_width), xzdd);
  vec2 anti_aliasing = xzdd*1.5f;
	vec2 grid = 1.0f - abs(mod(world_position.xz, cell_size)*2.0f - 1.0f);
	grid = smoothstep(draw_width + anti_aliasing, draw_width - anti_aliasing, grid);
	grid *= clamp(line_width / draw_width, 0.0f, 1.0f);
	float grid_value = mix(grid.x, 1.0f, grid.y);
	return grid_value;
}

void main(void)
{
  float line_width = 0.01;

  vec3 x_axis_color = vec3(0.83f, 0.1f, 0.2f);
  vec3 z_axis_color = vec3(0.12f, 0.24f, 0.78f);

	vec2 xzdd = fwidth(world_position.xz);
  vec2 anti_aliasing = xzdd*1.5f;

  vec2 axis_draw_width = max(vec2(line_width), xzdd);
  vec2 axis = 1.0f - smoothstep(axis_draw_width - anti_aliasing, axis_draw_width + anti_aliasing, abs(world_position.xz));
  float axis_value = mix(axis.x, 1.0f, axis.y);
  vec3 axis_color = mix(x_axis_color, z_axis_color, axis.x);

	// --AlNov: @TODO To understand what is going on there :)
  float grid_size = 1.0f;
	float grid_value = DrawGrid(grid_size, line_width);
	float subgrid_value = DrawGrid(0.1f, .01f);

  frag_color = vec4(axis_color, axis_value);

	frag_color.rgb = mix(grid_color.rgb, axis_color, axis_value);
	frag_color.a = max(grid_value, subgrid_value*.5f);
}
