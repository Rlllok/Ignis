#version 460

layout(location = 0) in struct FSData
{
    vec2 mouse_position;
    float time;
    float dt;
} fs_data;
  
layout(location = 0) out vec4 out_color;

float Point(vec2 p, vec2 c)
{
  float d = length(p - c);
  float r = 25.0f;
  return smoothstep(fwidth(d), 5.0f, r - d);
}

float Line(vec2 p, vec2 a, vec2 b)
{
  vec2 pa = p - a;
  vec2 ba = b - a;
  float t = clamp(dot(pa, ba) / dot(ba, ba), 0.0f, 1.0f);
  vec2 c = a + ba*t;
  float d = length(c - p);
  return 1 - smoothstep(fwidth(d), 0.0f, 2.0f - d);
}

vec2 Bezier(vec2 a, vec2 b, vec2 c, float t)
{
  return mix(mix(a, c, t), mix(c, b, t), t);
}

void main()
{
  int num_segs = 25;
  
  vec2 a = vec2(100.0f, 100.0f);
  vec2 b = vec2(1100.0f, 600.0f);
  vec2 cp0 = fs_data.mouse_position;

  float t = (sin(fs_data.time) + 1.0f) * 0.5f;
  vec2 a_cp0 = mix(a, cp0, t);
  vec2 cp0_b = mix(cp0, b, t);
  vec2 curve_p = mix(a_cp0, cp0_b, t);

  vec3 color = vec3(0.0f);

  color += vec3(1.0f, 0.0f, 0.0f) * Point(gl_FragCoord.xy, a);
  color += vec3(0.0f, 0.0f, 1.0f) * Point(gl_FragCoord.xy, b);
  color += vec3(0.0f, 1.0f, 0.0f) * Point(gl_FragCoord.xy, cp0);
  // color += vec3(0.0f, 1.0f, 1.0f) * Point(gl_FragCoord.xy, a_cp0);
  // color += vec3(1.0f, 1.0f, 0.0f) * Point(gl_FragCoord.xy, cp0_b);
  color += vec3(0.2f, 0.4f, 0.6f) * Point(gl_FragCoord.xy, curve_p);
  
  color += Line(gl_FragCoord.xy, a, cp0);
  color += Line(gl_FragCoord.xy, cp0, b);
  color += Line(gl_FragCoord.xy, a_cp0, cp0_b);

  vec2 prev_p = a;
  for (int i = 1; i < num_segs + 1; i += 1)
  {
    float t = float(i) / float(num_segs);
    vec2 p = Bezier(a, b, cp0, t);

    color = max(color, vec3(0.2f, 0.4f, 0.6f) * Line(gl_FragCoord.xy, prev_p, p));
    
    prev_p = p;
  }
  
  out_color = vec4(color, 1.0f);
}
