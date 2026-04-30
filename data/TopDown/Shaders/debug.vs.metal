using namespace metal;

struct VertexShaderInput {
  float3 position [[attribute(0)]];
};

struct InstanceUniformData {
  float4x4 transform;
};

struct InstanceDataBuffer {
  device InstanceUniformData* uniform [[id(0)]];
};

vertex float4 VertexMain(
  VertexShaderInput input [[stage_in]],
  constant InstanceDataBuffer& instance_data_buffer [[buffer(3)]]
) {
  return instance_data_buffer.uniform->transform*float4(input.position, 1.0f);
}
