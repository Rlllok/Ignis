#version 460

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec3 fragPosition;
layout(location = 3) in vec2 fragUV;

layout(binding = 1) uniform sampler2D texture_sampler;

layout(location = 0) out vec4 outColor;

void main()
{
  vec3 final_color = vec3(0.0f);

  vec3 red    = vec3(1.0f, 0.0f, 0.0f);
  vec3 blue   = vec3(0.0f, 0.0f, 1.0f);
  vec3 white  = vec3(1.0f, 1.0f, 1.0f);

  float horizontal_line      = smoothstep(0.0f, 0.005f, abs(fragUV.y - 0.5));
  float linear_function_line = smoothstep(0.0f, 0.005f, abs(fragUV.y - mix(0.05f, 1.0f, fragUV.x)));

  if (fragUV.y > 0.5f)
  {
    final_color = mix(red, blue, fragUV.x);
  }
  else
  {
    final_color = mix(red, blue, smoothstep(0.0f, 1.0f, fragUV.x));
  }

  final_color = mix(white, final_color, horizontal_line);

  outColor = vec4(final_color, 1.0f);
}
