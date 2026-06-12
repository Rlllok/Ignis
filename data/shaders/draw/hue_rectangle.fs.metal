using namespace metal;

struct RectangleData {
  float4x4 projection;
  float4   position_size;
  float    hue;
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
  float4 color = float4(RGBFromHSV(uv.x, 1.0f, 1.0f), 0.0f);
  
  float hue_rectangle = SDF_Rectangle(input.local_xy - input.half_size, input.half_size);
  hue_rectangle = 1.0f - smoothstep(0.0f, fwidth(hue_rectangle), hue_rectangle);
  color = mix(color, float4(RGBFromHSV(uv.x, 1.0f, 1.0f), 1.0f), hue_rectangle);

  float2 pointer_half_size = float2(2.0f, input.half_size.y*2.0f);
  float2 pointer_position = float2(input.local_xy.x - 2.0f*input.half_size.x*rectangle.hue, input.local_xy.y);
  float pointer = SDF_Rectangle(pointer_position, pointer_half_size);
  pointer = 1.0f - smoothstep(0.0f, fwidth(pointer), pointer);
  color = mix(color, float4(1.0f, 1.0f, 1.0f, 1.0f), pointer);
  
  return color;
}
