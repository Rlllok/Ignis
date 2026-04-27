using namespace metal;

fragment float4 FragmentMain(float4 input [[stage_in]]) {
  return float4(1.0f, 0.0f, 0.0f, 0.3f);
}
