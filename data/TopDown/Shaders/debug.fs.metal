using namespace metal;

struct VertexOut {
  float4 position [[position]];
  float4 rgba [[flat]];
};

fragment float4 FragmentMain(
  VertexOut input [[stage_in]]
) {
  return input.rgba;
}
