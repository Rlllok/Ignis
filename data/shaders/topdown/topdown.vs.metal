using namespace metal;

struct VertexShaderInput {
  float3 position [[attribute(0)]];
  float3 normal   [[attribute(1)]];
  float2 tangent  [[attribute(2)]];
  float2 uv       [[attribute(3)]];
};

struct InstanceUniforms {
  float4x4 mvp;
};

struct InstanceDataBuffer {
  device InstanceUniforms* uniform [[id(0)]];
};

struct VertexOut {
  float4 position [[position]];
  float3 local_position;
  float2 uv;
};

vertex VertexOut VertexMain(
  VertexShaderInput vertex_data [[stage_in]],
  constant InstanceDataBuffer& instance_data_buffer [[buffer(3)]],
  uint vid [[vertex_id]]
) {
  VertexOut out = {0};

  out.local_position = vertex_data.position;
  out.position = instance_data_buffer.uniform->mvp*float4(vertex_data.position, 1.0f);
  out.uv = vertex_data.uv;

  return out;
}
