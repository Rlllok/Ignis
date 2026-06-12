using namespace metal;

struct RectangleData {
  float4x4 projection;
  float4   position_size;
  float3   hsv;
};

struct VertexOutput {
  float4 position [[position]];
  float2 half_size [[flat]];
  float2 local_xy;
};

float SDF_Rectangle(float2 position, float2 half_size) {
  float2 d = abs(position) - half_size;
  return length(max(d, 0.0f)) + min(max(d.x, d.y), 0.0f);
}

float SDF_Circle(float2 position, float2 center, float radius) {
  return length(position - center) - radius;
}

float3 RGBFromHSV(float hue, float saturation, float value) {
  float3 result = float3(0.0f);
  float k = fmod(5.0f + hue*6.0f, 6.0f);
  result.r = value - value*saturation*max(0.0f, min(min(k, 4.0f - k), 1.0f));
  k = fmod(3.0f + hue*6.0f, 6.0f);
  result.g = value - value*saturation*max(0.0f, min(min(k, 4.0f - k), 1.0f));
  k = fmod(1.0f + hue*6.0f, 6.0f);
  result.b = value - value*saturation*max(0.0f, min(min(k, 4.0f - k), 1.0f));
  return result;
}

fragment float4 FragmentMain(
  VertexOutput input [[stage_in]],
  constant RectangleData* rectangle_data [[buffer(1)]]
) {
  RectangleData rectangle = rectangle_data[0];

  float2 uv = input.local_xy/(input.half_size*2.0f);
  float4 color = float4(0.0f, 0.0f, 0.0f, 0.0f);

  float value_saturation_zone = SDF_Rectangle(input.local_xy - input.half_size, input.half_size);
  value_saturation_zone = 1.0f - smoothstep(0.0f, fwidth(value_saturation_zone), value_saturation_zone);
  color = mix(color, float4(RGBFromHSV(rectangle.hsv.x, uv.x, (1.0f - uv.y)), 1.0f), value_saturation_zone);

  float2 center = float2(rectangle.hsv.y, 1.0f - rectangle.hsv.z)*input.half_size*2.0f;
  float circle = SDF_Circle(input.local_xy, center, 5.0f);
  float circle_t = 1.0f - smoothstep(0.0f, fwidth(circle), circle);

  color = mix(color, float4(1.0f, 1.0f, 1.0f, 1.0f), circle_t);
  
  return color;
}
