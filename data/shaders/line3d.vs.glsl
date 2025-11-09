#version 460

int indecies[6] = {0,1,3,1,2,3};

layout(set = 0, binding = 0) uniform GlobalData
{
  mat4 view_matrix;
  mat4 projection_matrix;
  vec3 camera_direction;
};

layout(set = 1, binding = 0) uniform InstanceData
{
  vec4 line_color;
  vec3 line_start;
  float line_width;
  vec3 line_end;
};

layout(location = 0) out vec4 out_color;

void main(void)
{
  vec3 line_direction = normalize(line_end - line_start);
  vec3 width_direction = normalize(cross(camera_direction, line_direction));

  vec3 vertex_positions[4] = {
    line_end + width_direction*line_width,
    line_start + width_direction*line_width,
    line_start - width_direction*line_width,
    line_end - width_direction*line_width,
  };

  gl_Position = projection_matrix*view_matrix*vec4(vertex_positions[indecies[gl_VertexIndex]], 1.0f);
  out_color = line_color;
}
