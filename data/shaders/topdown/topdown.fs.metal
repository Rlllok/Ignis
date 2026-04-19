using namespace metal;

struct VertexOut {
  float4 position [[position]];
  float3 local_position;
  float2 uv;
};

struct GlobalDataBuffer {
  texture2d<float> color_texture [[id(0)]];
};

fragment float4 FragmentMain(
  VertexOut input [[stage_in]],
  constant GlobalDataBuffer& global_data_buffer [[buffer(2)]]
) {
  sampler s(mag_filter::linear, min_filter::linear);
  return global_data_buffer.color_texture.sample(s, input.uv);
}
