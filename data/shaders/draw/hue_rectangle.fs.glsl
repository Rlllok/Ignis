#version 460

#extension GL_EXT_buffer_reference    : require
#extension GL_EXT_buffer_reference2   : require

float SDF_Rectangle(vec2 position, vec2 half_size) {
  vec2 d = abs(position) - half_size;
  return length(max(d, 0.0f)) + min(max(d.x, d.y), 0.0f);
}

vec3 RGBFromHSV(float hue, float saturation, float value) {
  vec3 result = vec3(0.0f);
  float k = mod(5.0f + hue*6.0f, 6.0f);
  result.r = value - value*saturation*max(0.0f, min(min(k, 4.0f - k), 1.0f));
  k = mod(3.0f + hue*6.0f, 6.0f);
  result.g = value - value*saturation*max(0.0f, min(min(k, 4.0f - k), 1.0f));
  k = mod(1.0f + hue*6.0f, 6.0f);
  result.b = value - value*saturation*max(0.0f, min(min(k, 4.0f - k), 1.0f));
  return result;
}

layout(buffer_reference, std430) readonly buffer RectangleDataBuffer {
  mat4x4 projection;
  vec4   position_size;
  float  hue;
};

layout(push_constant, std430) uniform args {
  RectangleDataBuffer rectangle_data;
};

layout(location = 0) in flat vec2 half_size;
layout(location = 1) in      vec2 local_xy;

layout(location = 0) out vec4 color_attachment;

void main() {
  vec2 uv = local_xy/(half_size*2.0f);
  vec4 color = vec4(RGBFromHSV(uv.x, 1.0f, 1.0f), 0.0f);
  
  float hue_rectangle = SDF_Rectangle(local_xy - half_size, half_size);
  hue_rectangle = 1.0f - smoothstep(0.0f, fwidth(hue_rectangle), hue_rectangle);
  color = mix(color, vec4(RGBFromHSV(uv.x, 1.0f, 1.0f), 1.0f), hue_rectangle);

  vec2 pointer_half_size = vec2(2.0f, half_size.y*2.0f);
  vec2 pointer_position = vec2(local_xy.x - 2.0f*half_size.x*rectangle_data.hue, local_xy.y);
  float pointer = SDF_Rectangle(pointer_position, pointer_half_size);
  pointer = 1.0f - smoothstep(0.0f, fwidth(pointer), pointer);
  color = mix(color, vec4(1.0f, 1.0f, 1.0f, 1.0f), pointer);

  color_attachment = color;
}
