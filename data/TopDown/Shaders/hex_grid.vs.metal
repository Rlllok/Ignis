using namespace metal;

constant float4 vertecies[] = {
  float4(-1.0f, 0.0f, -1.0f, 1.0f),
  float4( 1.0f, 0.0f, -1.0f, 1.0f),
  float4( 1.0f, 0.0f,  1.0f, 1.0f),
  float4(-1.0f, 0.0f,  1.0f, 1.0f),
};

constant uint indecies[] = {
  0, 1, 3,
  1, 2, 3
};

struct GridData {
  float4x4 transform;
  float4x4 camera_transform;
  float3   background_color;
  float3   grid_color;
};

struct VertexOutput {
  float4 position [[position]];
  float3 world_position;
};

vertex VertexOutput VertexMain(
  uint vertex_index [[vertex_id]],
  constant GridData* grid_data [[buffer(1)]]
) {
  VertexOutput output = {0};

  output.world_position = float3(grid_data->transform*float4(vertecies[indecies[vertex_index]])); 
  output.position = grid_data->camera_transform*float4(output.world_position, 1.0f);

  return output;
}
