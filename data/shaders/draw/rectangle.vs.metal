using namespace metal;

constant float4 vertecies[] = {
  float4(0.0f, 1.0f, 0.0f, 1.0f),
  float4(1.0f, 1.0f, 0.0f, 1.0f),
  float4(1.0f, 0.0f, 0.0f, 1.0f),
  float4(0.0f, 0.0f, 0.0f, 1.0f),
};

constant uint indecies[] = {
  0, 1, 3,
  1, 2, 3
};

struct RectangleData {
  float4x4 projection;
  float4   position_size;
  float4   color;
};

struct VertexOutput {
  float4 position [[position]];
};

vertex VertexOutput VertexMain(
  uint vertex_index [[vertex_id]],
  constant RectangleData* rectangle_data [[buffer(1)]]
) {
  VertexOutput output = {0};

  RectangleData rectangle = rectangle_data[0];

  float4 vertex_position = vertecies[indecies[vertex_index]]*float4(rectangle.position_size.z, rectangle.position_size.w, 0.0f, 1.0f) + float4(rectangle.position_size.x, rectangle.position_size.y, 0.0f, 0.0f);

  output.position = rectangle.projection*vertex_position;

  return output;
}
