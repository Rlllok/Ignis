#version 460

#extension GL_EXT_buffer_reference    : require
#extension GL_EXT_buffer_reference2   : require

float HexagonSDF(vec2 uv) {
  uv = abs(uv);
  vec2 k = vec2(0.5f, 0.886f);
  return max(dot(uv, k), uv.x);
}

float HexagonGrid(vec2 uv) {
  vec2 k = vec2(1.0f, 1.732f);
  vec4 hex_center = round(vec4(uv, uv - vec2(0.5f, 1.0f))/k.xyxy);
  vec4 new_center = vec4(uv - hex_center.xy*k, uv - (hex_center.zw + 0.5f)*k);
  vec2 result = vec2(0.0f, 0.0f);
  if (dot(new_center.xy, new_center.xy) < dot(new_center.zw, new_center.zw)) {
    result = new_center.xy;
  }
  else {
    result = new_center.zw;
  }
  return HexagonSDF(result);
}

layout(location = 0) in vec3 world_position;

layout(buffer_reference, std430) readonly buffer GridDataBuffer {
  mat4x4 transform;
  mat4x4 camera_transform;
  vec3 background_color;
  vec3 grid_color;
};

layout(push_constant, std430) uniform args {
  GridDataBuffer grid_data;
};

layout(location = 0) out vec4 color_attachment;

void main() {
  float grid_size = 100.0f;
  vec2 uv = 50.0f*world_position.xz/grid_size;
  float line_width = 0.02f;
  float grid_cell = HexagonGrid(uv);
  float grid_border = 0.8f*smoothstep(-0.02, 0.0f, grid_cell - 0.5f + line_width);
  float grid_center = 0.3f*smoothstep(0.3f + 0.02f, 0.3f, grid_cell);

  vec3 grid = grid_data.background_color;
  grid = mix(grid, grid_data.grid_color, grid_border);
  grid = mix(grid, grid_data.grid_color, grid_center);
  color_attachment = vec4(grid, 1.0f);
}
