#version 460

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec3 fragPosition;
layout(location = 3) in vec2 frag_uv;

layout(location = 0) out vec4 out_color;

void main()
{
  vec3 final_color = vec3(0.0f);

  vec3 red    = vec3(1.0f, 0.0f, 0.0f);
  vec3 blue   = vec3(0.0f, 0.0f, 1.0f);
  vec3 white  = vec3(1.0f, 1.0f, 1.0f);
  vec3 yellow = vec3(1.0f, 1.0f, 0.0f);

  float horizontal_line      = smoothstep(0.0f, 0.0075f, abs(frag_uv.y - 0.5));
  float linear_function_line = smoothstep(0.0f, 0.0025f, abs(frag_uv.y - mix(0.5f, 1.0f, frag_uv.x)));
  float curve_function_line  = smoothstep(0.0f, 0.0025f, abs(frag_uv.y - mix(0.0f, 0.5f, smoothstep(0.0f, 1.0f, frag_uv.x))));

  if (frag_uv.y > 0.5f)
  {
    final_color = mix(red, blue, frag_uv.x);
  }
  else
  {
    final_color = mix(red, blue, smoothstep(0.0f, 1.0f, frag_uv.x));
  }

  final_color = mix(white, final_color, horizontal_line);
  final_color = mix(yellow, final_color, linear_function_line);
  final_color = mix(yellow, final_color, curve_function_line);

  out_color = vec4(final_color, 1.0f);
}