using namespace metal;

struct VertexIn {
  float4 position [[attribute(0)]];
};

struct InstanceUniformData {
  float3 translation;
};

struct IntanceDataBuffer {
  device InstanceUniformData* uniform [[id(0)]];
};

struct VertexOut {
  float4 position [[position]];
  float4 color;
};

vertex VertexOut vertex_main(
  VertexIn vertex_data [[stage_in]],
  constant IntanceDataBuffer& instance_data_buffer [[buffer(3)]],
  uint vid [[vertex_id]]
) {
  const float3 colors[3] = {
      float3(1.0, 0.0, 0.0), // Red
      float3(0.0, 1.0, 0.0), // Green
      float3(0.0, 0.0, 1.0)  // Blue
  };

  VertexOut out = {0};
  out.position = vertex_data.position + float4(instance_data_buffer.uniform->translation, 0.0f);
  out.color = float4(colors[vid], 1.0);
  return out;
}
