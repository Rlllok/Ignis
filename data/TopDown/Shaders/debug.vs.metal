using namespace metal;

constant float4 vertecies[] = {
  float4(-0.5f, 0.5f, 0.5f, 1.0f),
  float4(-0.5f, 0.5f,-0.5f, 1.0f),
  float4( 0.5f, 0.5f,-0.5f, 1.0f),
  float4( 0.5f, 0.5f, 0.5f, 1.0f),

  float4(-0.5f,-0.5f, 0.5f, 1.0f),
  float4(-0.5f,-0.5f,-0.5f, 1.0f),
  float4( 0.5f,-0.5f,-0.5f, 1.0f),
  float4( 0.5f,-0.5f, 0.5f, 1.0f),
};

constant uint indecies[] = {
  0, 1, 2,
  0, 2, 3,
  
  4, 5, 6,
  4, 6, 7,

  0, 1, 5,
  0, 5, 4,

  3, 2, 6,
  3, 6, 7,

  0, 3, 4,
  0, 7, 4,

  1, 2, 6,
  1, 6, 5,
};

struct BoundingBox {
  float4x4 transform;
  float4x4 camera_transform;
  float4   rgba;
};

struct VertexOut {
  float4 position [[position]];
  float4 rgba [[flat]];
};

vertex VertexOut VertexMain(
  uint vertex_index [[vertex_id]],
  uint instance_index [[instance_id]],
  constant BoundingBox* bounding_boxes[[buffer(1)]]
) {
  BoundingBox bounding_box = bounding_boxes[instance_index];

  VertexOut out = {0};
  out.position = bounding_box.camera_transform*bounding_box.transform*vertecies[indecies[vertex_index]];
  out.rgba = bounding_box.rgba;
  return out;
}
