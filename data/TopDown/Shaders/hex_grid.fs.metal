using namespace metal;

struct VertexOutput {
  float4 position [[position]];
  float3 world_position;
};

float HexagonSDF(float2 uv) {
  uv = abs(uv);
  const float2 k = float2(0.5f, 0.886f);
  return max(dot(uv, k), uv.x);
}

float HexagonGrid(float2 uv) {
  float2 k = float2(1.0f, 1.732f);
  float4 hex_center = round(float4(uv, uv - float2(0.5f, 1.0f))/k.xyxy);
  float4 new_center = float4(uv - hex_center.xy*k, uv - (hex_center.zw + 0.5f)*k);
  float2 result = float2(0.0f, 0.0f);
  if (dot(new_center.xy, new_center.xy) < dot(new_center.zw, new_center.zw)) {
    result = new_center.xy;
  }
  else {
    result = new_center.zw;
  }
  return HexagonSDF(result);
}

struct GridData {
  float4x4 transform;
  float4x4 camera_transform;
  float4   background_color;
  float4   grid_color;
};

fragment float4 FragmentMain(
  VertexOutput input [[stage_in]],
  constant GridData* grid_data [[buffer(1)]]
) {
  float grid_size = 100.0f;
  float2 uv = 50.0f*input.world_position.xz/grid_size;
  float line_width = 0.02f;
  float grid_cell = HexagonGrid(uv);
  float grid_border = 0.8f*smoothstep(-0.02, 0.0f, grid_cell - 0.5f + line_width);
  float grid_center = 0.3f*smoothstep(0.3f + 0.02f, 0.3f, grid_cell);

  float4 grid = grid_data->background_color;
  grid = mix(grid, grid_data->grid_color, grid_border);
  grid = mix(grid, grid_data->grid_color, grid_center);
  return grid;
}
