using namespace metal;

struct VertexIn {
  float4 position [[attribute(0)]];
};

struct Material {
  float4 color;
};

struct EntityData {
  float4 translation;
};

struct Arguments {
  device Material* materials;
  device EntityData* entity_datas;
};

struct VertexOut {
  float4 position [[position]];
  float4 color;
};

vertex VertexOut VertexMain(
  VertexIn vertex_data [[stage_in]],
  uint vid [[vertex_id]],
  uint instance_index [[instance_id]],
  constant Material* materials [[buffer(1)]],
  constant EntityData* entity_datas [[buffer(2)]]
) {
  Material material = materials[instance_index];

  VertexOut out = {0};
  out.position = vertex_data.position + entity_datas[instance_index].translation;
  out.color = material.color;
  return out;
}
