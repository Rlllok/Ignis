#version 460

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec3 fragPosition;
layout(location = 3) in vec2 frag_uv;

layout(location = 0) out vec4 out_color;

float IG_Math_Random(vec2 p)
{
  p = 50.0f * fract(p * 0.3183099 + vec2(0.71f, 0.113f));
  return -1.0f + 2.0f * fract(p.x * p.y * (p.x + p.y));
}

vec4 IG_Noise(vec2 coords)
{
  vec2 tex_size = vec2(1.0f);
  vec2 pc = coords * tex_size;
  vec2 base = floor(pc);

  float s1 = IG_Math_Random((base + vec2(0.0f, 0.0f)) / tex_size);
  float s2 = IG_Math_Random((base + vec2(1.0f, 0.0f)) / tex_size);
  float s3 = IG_Math_Random((base + vec2(0.0f, 1.0f)) / tex_size);
  float s4 = IG_Math_Random((base + vec2(1.0f, 1.0f)) / tex_size);

  vec2 f = smoothstep(0.0f, 1.0f, fract(pc));

  float px1 = mix(s1, s2, f.x);
  float px2 = mix(s3, s4, f.x);
  float result = mix(px1, px2, f.y);
  return vec4(result);
}

void main()
{
  out_color = vec4(IG_Noise(frag_uv * 10.0f));
}