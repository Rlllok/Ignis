using namespace metal;

struct RectangleData {
  float4x4 projection;
  float4   position_size;
  float4   color;
};

struct VertexOutput {
  float4 position [[position]];
};

fragment float4 FragmentMain(
  VertexOutput input [[stage_in]],
  constant RectangleData* rectangle_data [[buffer(1)]]
) {
  RectangleData rectangle = rectangle_data[0];
  
  return rectangle.color;
}
