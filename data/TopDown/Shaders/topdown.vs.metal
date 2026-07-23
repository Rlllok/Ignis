using namespace metal;

struct VertexShaderInput {
  float3 position [[attribute(0)]];
  float3 normal   [[attribute(1)]];
  float3 tangent  [[attribute(2)]];
  float2 uv       [[attribute(3)]];
  int4   joint_ids [[attribute(4)]];
  float4 jount_weights [[attribute(5)]];
};

struct SceneData {
  float3 light_direction;
  float3 light_color;
};

struct Material {
  packed_float3 color;
};

struct ObjectData {
  float4x4 transform;
  float4x4 camera_transform;
  Material material;
};

struct VertexOut {
  float4 position [[position]];
  float3 world_position;
  float3 normal;
  float3 color [[flat]];
};

vertex VertexOut VertexMain(
  VertexShaderInput vertex_data [[stage_in]],
  uint instance_index [[instance_id]],
  constant SceneData* scene_data [[buffer(1)]],
  constant ObjectData* object_datas [[buffer(2)]]
) {
  VertexOut out = {0};

  ObjectData current_object = object_datas[instance_index];

  out.world_position = float3(current_object.transform*float4(vertex_data.position, 1.0f));
  out.position = current_object.camera_transform*float4(out.world_position, 1.0f);
  out.normal = vertex_data.normal;
  out.color = current_object.material.color;

  return out;
}
