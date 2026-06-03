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
  float4   radius;
  float4   color;
};

struct VertexOutput {
  float4 position [[position]];
  float2 half_size [[flat]];
  float4 radius [[flat]];
  float2 local_xy;
};

vertex VertexOutput VertexMain(
  uint vertex_index [[vertex_id]],
  constant RectangleData* rectangle_data [[buffer(1)]]
) {
  VertexOutput output = {0};

  RectangleData rectangle = rectangle_data[0];

  float4 vertex_position = vertecies[indecies[vertex_index]]*float4(rectangle.position_size.z, rectangle.position_size.w, 0.0f, 1.0f) + float4(rectangle.position_size.x, rectangle.position_size.y, 0.0f, 0.0f);

  output.position = rectangle.projection*vertex_position;
  output.half_size  = rectangle.position_size.zw/2.0f;
  output.radius = clamp(rectangle.radius, 0.0f, min(output.half_size.x, output.half_size.y));
  output.local_xy = vertex_position.xy - rectangle.position_size.xy;

  return output;
}
