using namespace metal;

struct VertexOut {
  float4 position [[position]];
  float3 world_position;
  float3 normal;
  float3 color [[flat]];
};

struct SceneData {
  float3 light_direction;
  float3 light_color;
};

fragment float4 FragmentMain(
  VertexOut input [[stage_in]],
  constant SceneData* scene_data [[buffer(1)]]
) {
  float diffuse_factor = max(dot(input.normal, -scene_data->light_direction), 0.0f);

  return float4(diffuse_factor*input.color, 1.0f);
}
