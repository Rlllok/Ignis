using namespace metal;

struct VertexOut {
  float4 position [[position]];
  float3 local_position;
};

fragment float4 FragmentMain(VertexOut input [[stage_in]]) {
  return float4(input.local_position, 1.0f);
}
