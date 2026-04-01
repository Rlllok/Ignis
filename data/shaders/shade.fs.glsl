#version 460

layout(set = 2, binding = 0) uniform GlobalData {
  vec2 resolution;
};

layout(location = 0) in vec2 in_uv;

layout(location = 0) out vec4 out_color;

vec3 color_palette(vec3 color_a, vec3 color_b, vec3 color_c, vec3 color_d, float t) {
  return color_a + color_b*cos(6.2831*(color_c*t + color_d));
}

float plot(float t, float f) {
  return smoothstep(0.01, 0.0, abs(t - f));
}

vec3 cosine_palette_example(vec2 uv) {
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

vec3 grid_example(vec2 uv) {
  vec2 aspect_ratio = resolution/min(resolution.x, resolution.y);
  uv = (uv - 0.5f)*2.0f*aspect_ratio;
  vec3 pixel = vec3(0.92f, 0.90f, 0.90f);
  vec3 grid_color = vec3(0.0f, 0.0f, 0.0f);
  float grid_interval = 0.1f;
  float grid_width = 0.003f;
  vec3 axis_color = vec3(1.0f, 0.0f, 0.0f);
  float axis_width = 0.005f;

  if (mod(uv.x, grid_interval) < grid_width) {
    pixel = grid_color;
  }
  if (mod(uv.y, grid_interval) < grid_width) {
    pixel = grid_color;
  }

  if (abs(uv.x) < axis_width) {
    pixel = axis_color;
  }
  if (abs(uv.y) < axis_width) {
    pixel = axis_color;
  }

  return pixel;
}

vec3 clamp_example(vec2 uv) {
  vec3 pixel = vec3(0.0f, 0.0f, 0.0f);

  float t = cos(2.0f*3.14*uv.y*8.0f);
  pixel = vec3(clamp(t, 0.1f, 0.8f));

  return pixel;
}

vec3 smoothstep_example(vec2 uv) {
  vec3 pixel = vec3(0.0f, 0.0f, 0.0);

  if (uv.x < 0.5) {
    float f = clamp((uv.y - 0.35f)/(0.55f - 0.35f), 0.0f, 1.0f);
    pixel = vec3(f);
  }
  else {
    pixel = vec3(smoothstep(0.35f, 0.55f, uv.y));
  }

  return pixel;
}

void main() {
  vec2 uv = gl_FragCoord.xy/resolution;

  // out_color = vec4(cosine_palette_example(uv), 1.0f);
  // out_color = vec4(grid_example(in_uv), 1.0f);
  // out_color = vec4(clamp_example(in_uv), 1.0f);
  out_color = vec4(smoothstep_example(in_uv), 1.0f);
}
