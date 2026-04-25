using namespace metal;

struct VertexOut {
  float4 position [[position]];
  float3 world_position;
  float3 normal;
  float3 color [[flat]];
};

struct Light {
  float3 direction;
  float3 color;
};

struct GlobalUniforms {
  Light light;
};

struct GlobalDataBuffer {
  device GlobalUniforms* global_uniforms [[id(0)]];
};

fragment float4 FragmentMain(
  VertexOut input [[stage_in]],
  constant GlobalDataBuffer& global_data_buffer [[buffer(2)]]
) {
  float3 ambient = 0.1f*global_data_buffer.global_uniforms->light.color;

  float3 normal = normalize(input.normal);
  float3 diffuse = max(dot(normal, -global_data_buffer.global_uniforms->light.direction), 0.0f)*global_data_buffer.global_uniforms->light.color;
  return float4((ambient + diffuse)*input.color, 1.0f);
}
