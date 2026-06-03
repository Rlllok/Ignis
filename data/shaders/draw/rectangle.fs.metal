using namespace metal;

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

float SDF_Rectangle(float2 position, float2 half_size, float4 radius) {
  // check in what corner of rectangle the point is located
  float2 r = (position.x < 0.0f) ? radius.xy : radius.zw;
  r.x = (position.y < 0.0f) ? r.x : r.y;
  float2 d = abs(position) - half_size + float(r.x);
  return length(max(d, 0.0f)) + min(max(d.x, d.y), 0.0f) - r.x;
}

fragment float4 FragmentMain(
  VertexOutput input [[stage_in]],
  constant RectangleData* rectangle_data [[buffer(1)]]
) {
  RectangleData rectangle = rectangle_data[0];

  float2 uv = (input.local_xy - input.half_size)/(input.half_size*2.0f);
  float2 uv_size = input.half_size/max(input.half_size.x, input.half_size.y);
  
  float d = SDF_Rectangle(input.local_xy - input.half_size, input.half_size, input.radius);
  float aa = fwidth(d);
  float t = 1.0f - smoothstep(-aa, aa, d);
  
  float4 color = rectangle.color;
  color.a *= t;
  return rectangle.color*t;
}
