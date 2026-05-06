using namespace metal;

struct VertexOut {
  float4 position [[position]];
  float4 color;
};

fragment float4 FragmentMain(VertexOut in [[stage_in]]) {
  return in.color;
}
