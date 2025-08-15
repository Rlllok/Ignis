#version 460

layout(set = 1, binding = 0) uniform GlobalData
{
	vec4 grid_color;
};

layout(location = 0) in vec3 world_position;

layout(location = 0) out vec4 frag_color;

void main(void)
{
	float cell_size = 1.0f;
	float cell_halfsize = cell_size*0.5f;
	float grid_line_width = 0.01f;

	float subcell_size = cell_size/10.0f;
	float subcell_halfsize = subcell_size*0.5f;
	float subgrid_line_width = 0.007f;

	vec3 grid = vec3(0.0f);
	grid = mod(world_position - cell_halfsize, cell_size);
	grid = abs(grid - cell_halfsize);
	grid = smoothstep(grid_line_width, 0.0f, grid);

	float subgrid_x = 0;
	subgrid_x = mod(world_position.x - subcell_halfsize, subcell_size);
	subgrid_x = abs(subgrid_x - subcell_halfsize);
	subgrid_x = smoothstep(subgrid_line_width, 0.0f, subgrid_x);
	float subgrid_z = 0;
	subgrid_z = mod(world_position.z - subcell_halfsize, subcell_size);
	subgrid_z = abs(subgrid_z - subcell_halfsize);
	subgrid_z = smoothstep(subgrid_line_width, 0.0f, subgrid_z);
	vec3 subgrid = vec3(0.0f);
	subgrid = mod(world_position - subcell_halfsize, subcell_size);
	subgrid = abs(subgrid - subcell_halfsize);
	subgrid = smoothstep(subgrid_line_width, 0.0f, subgrid);
	
	float grid_value = max(max(grid.x, grid.z), max(subgrid.x, subgrid.z)*0.5f);

	frag_color = grid_color;
	frag_color.a = grid_value;
}
