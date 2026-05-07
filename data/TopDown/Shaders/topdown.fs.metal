using namespace metal;

struct VertexOut {
  float4 position [[position]];
  float3 world_position;
  float3 normal;
  float3 color [[flat]];
};

fragment float4 FragmentMain(
  VertexOut input [[stage_in]]
) {
  return float4(input.color, 1.0f);
}
