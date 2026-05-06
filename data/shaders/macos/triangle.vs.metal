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
  constant Arguments& args [[buffer(4)]]
) {

  VertexOut out = {0};
  out.position = vertex_data.position + args.entity_datas[instance_index].translation;
  if((uint64_t)args.materials < 0x1000 || (uint64_t)args.materials > 0x7FFFFFFFFFFF) {
    out.color = float4(0, 1, 0, 1);
  }
  else {
    Material material = args.materials[instance_index];
    out.color = material.color;
  }
  return out;
}
