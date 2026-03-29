#version 460

layout(location = 0) in in_value {
  vec2 uv;
};

layout(location = 0) out vec4 out_color;

vec3 color_palette(vec3 color_a, vec3 color_b, vec3 color_c, vec3 color_d, float t) {
  return color_a + color_b*cos(6.2831*(color_c*t + color_d));
}

float plot(float t, float f) {
  return smoothstep(0.01, 0.0, abs(t - f));
}

vec3 cosine_palette_example() {
  vec3 color = vec3(0.15f, 0.15f, 0.15f);

  vec3 palette = color_palette(vec3(0.5f, 0.5f, 0.5f), vec3(0.5f, 0.5f, 0.5f), vec3(2.0f, 1.5f, 0.0f), vec3(0.5f, 0.2f, 0.25f), uv.x);

  if (uv.y < 0.5f) {
    vec2 xy = uv*2.0;

    color = mix(color, vec3(1.0f, 0.0f, 0.0f)*palette.x, plot(xy.y, palette.x));
    color = mix(color, vec3(0.0f, 1.0f, 0.0f)*palette.y, plot(xy.y, palette.y));
    color = mix(color, vec3(0.0f, 0.0f, 1.0f)*palette.z, plot(xy.y, palette.z));
  }
  else {
    color = palette;
  }

  return color;
}

void main() {
  out_color = vec4(cosine_palette_example(), 1.0f);
}
