#version 460

#extension GL_EXT_buffer_reference    : require
#extension GL_EXT_buffer_reference2   : require

float SDF_Rectangle(vec2 position, vec2 half_size) {
  vec2 d = abs(position) - half_size;
  return length(max(d, 0.0f)) + min(max(d.x, d.y), 0.0f);
}

float SDF_Circle(vec2 position, vec2 center, float radius) {
  return length(position - center) - radius;
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
  vec3   hsv;
};

layout(push_constant, std430) uniform args {
  RectangleDataBuffer rectangle_data;
};

layout(location = 0) flat in vec2 half_size;
layout(location = 1)      in vec2 local_xy;

layout(location = 0) out vec4 color_attachment;

void main() {
  vec2 uv = local_xy/(half_size*2.0f);
  vec4 color = vec4(0.0f, 0.0f, 0.0f, 0.0f);

  float value_saturation_zone = SDF_Rectangle(local_xy - half_size, half_size);
  value_saturation_zone = 1.0f - smoothstep(0.0f, fwidth(value_saturation_zone), value_saturation_zone);
  color = mix(color, vec4(RGBFromHSV(rectangle_data.hsv.x, uv.x, (1.0f - uv.y)), 1.0f), value_saturation_zone);

  vec2 center = vec2(rectangle_data.hsv.y, 1.0f - rectangle_data.hsv.z)*half_size*2.0f;
  float circle = SDF_Circle(local_xy, center, 5.0f);
  float circle_t = 1.0f - smoothstep(0.0f, fwidth(circle), circle);

  color = mix(color, vec4(1.0f, 1.0f, 1.0f, 1.0f), circle_t);

  color_attachment = color;
}
