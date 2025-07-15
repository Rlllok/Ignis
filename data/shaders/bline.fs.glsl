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

vec2 CubicBezier(vec2 a, vec2 b, vec2 c0, vec2 c1, float t)
{
  vec2 ac0 = mix(a, c0, t);
  vec2 c0c1 = mix(c0, c1, t);
  vec2 c1b = mix(c1, b, t);
  return mix(mix(ac0, c0c1, t), mix(c0c1, c1b, t), t);
}

void main()
{
  int num_segs = 25;
  
  vec2 a = vec2(100.0f, 100.0f);
  vec2 b = vec2(1100.0f, 600.0f);
  vec2 c0 = fs_data.mouse_position;
  vec2 c1 = vec2(900.0f, 200.0f);

  float t = (sin(fs_data.time) + 1.0f) * 0.5f;
  vec2 ac0 = mix(a, c0, t);
  vec2 c0c1 = mix(c0, c1, t);
  vec2 c1b = mix(c1, b, t);
  vec2 curve_p = mix(mix(ac0, c0c1, t), mix(c0c1, c1b, t), t);

  vec3 color = vec3(0.0f);

  color += vec3(1.0f, 1.0f, 1.0f) * Point(gl_FragCoord.xy, a);
  color += vec3(1.0f, 1.0f, 1.0f) * Point(gl_FragCoord.xy, b);
  color += vec3(0.0f, 0.0f, 1.0f) * Point(gl_FragCoord.xy, c0);
  color += vec3(0.0f, 0.0f, 1.0f) * Point(gl_FragCoord.xy, c1);
  color += vec3(0.2f, 0.4f, 0.6f) * Point(gl_FragCoord.xy, curve_p);
  
  color += Line(gl_FragCoord.xy, a, c0);
  color += Line(gl_FragCoord.xy, c0, c1);
  color += Line(gl_FragCoord.xy, c1, b);
  color += vec3(0.0f, 1.0f, 0.0f) * Line(gl_FragCoord.xy, ac0, c0c1);
  color += vec3(0.0f, 1.0f, 0.0f) * Line(gl_FragCoord.xy, c0c1, c1b);
  color += vec3(1.0f, 0.0f, 0.0f) * Line(gl_FragCoord.xy, mix(ac0, c0c1, t), mix(c0c1, c1b,t));

  vec2 prev_p = a;
  for (int i = 1; i < num_segs + 1; i += 1)
  {
    float t = float(i) / float(num_segs);
    // vec2 p = Bezier(a, b, c0, t);
    vec2 p = CubicBezier(a, b, c0, c1, t);

    color = max(color, vec3(0.2f, 0.4f, 0.6f) * Line(gl_FragCoord.xy, prev_p, p));
    
    prev_p = p;
  }
  
  out_color = vec4(color, 1.0f);
}
