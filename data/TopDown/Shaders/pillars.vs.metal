using namespace metal;

constant float4 vertecies[] = {
  float4(-1.0f, 0.0f, -1.0f, 1.0f),
  float4( 1.0f, 0.0f, -1.0f, 1.0f),
  float4( 1.0f, 0.0f,  1.0f, 1.0f),
  float4(-1.0f, 0.0f,  1.0f, 1.0f),
};

constant float2 uv[] = {
  float2(0.0f, 1.0f),
  float2(1.0f, 1.0f),
  float2(1.0f, 0.0f),
  float2(0.0f, 0.0f),
};

constant uint indecies[] = {
  0, 1, 3,
  1, 2, 3
};

struct PillarsData {
  float3   player_position;
  float3   camera_position;
  float4x4 camera_inverse;
  float4   color;
  float2   resolution;
};

struct VertexOutput {
  float4 position [[position]];
};

vertex VertexOutput VertexMain(
  uint vertex_index [[vertex_id]],
  constant PillarsData* pillars_data [[buffer(1)]]
) {
  VertexOutput output = {0};

  output.position = float4(uv[indecies[vertex_index]]*2.0f - 1.0f, 1.0f, 1.0f);

  return output;
}
