using namespace metal;

struct VertexShaderInput {
  float3 position [[attribute(0)]];
  float3 normal   [[attribute(1)]];
};

struct Material {
  float3 color;
};

struct InstanceUniforms {
  float4x4 transform;
  float4x4 camera_transform;
  Material material;
};

struct InstanceDataBuffer {
  device InstanceUniforms* uniform [[id(0)]];
};

struct VertexOut {
  float4 position [[position]];
  float3 world_position;
  float3 normal;
  float3 color [[flat]];
};

vertex VertexOut VertexMain(
  VertexShaderInput vertex_data [[stage_in]],
  constant InstanceDataBuffer& instance_data_buffer [[buffer(3)]],
  uint vid [[vertex_id]]
) {
  VertexOut out = {0};

  out.world_position = float3(instance_data_buffer.uniform->transform*float4(vertex_data.position, 1.0f));
  out.position = instance_data_buffer.uniform->camera_transform*instance_data_buffer.uniform->transform*float4(vertex_data.position, 1.0f);
  out.normal = vertex_data.normal;
  out.color = instance_data_buffer.uniform->material.color;

  return out;
}
