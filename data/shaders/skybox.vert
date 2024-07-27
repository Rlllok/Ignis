#version 460

layout(binding = 0) uniform MVP
{
  vec3 color;
  mat4 view;
  mat4 translation;
} mvp;

layout(location = 0) out vec3 frag_color;
layout(location = 1) out vec3 frag_normal;
layout(location = 2) out vec3 frag_position;
layout(location = 3) out vec2 frag_uv;

vec3 vertices[36] = {
    vec3(-0.5f, -0.5f, -0.5f),
    vec3(0.5f, -0.5f, -0.5f),
    vec3(0.5f,  0.5f, -0.5f),
    vec3(0.5f,  0.5f, -0.5f),
    vec3(-0.5f,  0.5f, -0.5f),
    vec3(-0.5f, -0.5f, -0.5f),

    vec3(-0.5f, -0.5f,  0.5f),
    vec3(0.5f, -0.5f,  0.5f),
    vec3(0.5f,  0.5f,  0.5f),
    vec3(0.5f,  0.5f,  0.5f),
    vec3(-0.5f,  0.5f,  0.5f),
    vec3(-0.5f, -0.5f,  0.5f),

    vec3(-0.5f,  0.5f,  0.5f),
    vec3(-0.5f,  0.5f, -0.5f),
    vec3(-0.5f, -0.5f, -0.5f),
    vec3(-0.5f, -0.5f, -0.5f),
    vec3(-0.5f, -0.5f,  0.5f),
    vec3(-0.5f,  0.5f,  0.5f),

    vec3(0.5f,  0.5f,  0.5f),
    vec3(0.5f,  0.5f, -0.5f),
    vec3(0.5f, -0.5f, -0.5f),
    vec3(0.5f, -0.5f, -0.5f),
    vec3(0.5f, -0.5f,  0.5f),
    vec3(0.5f,  0.5f,  0.5f),

    vec3(-0.5f, -0.5f, -0.5f),
    vec3(0.5f, -0.5f, -0.5f),
    vec3(0.5f, -0.5f,  0.5f),
    vec3(0.5f, -0.5f,  0.5f),
    vec3(-0.5f, -0.5f,  0.5f),
    vec3(-0.5f, -0.5f, -0.5f),

    vec3(-0.5f,  0.5f, -0.5f),
    vec3(0.5f,  0.5f, -0.5f),
    vec3(0.5f,  0.5f,  0.5f),
    vec3(0.5f,  0.5f,  0.5f),
    vec3(-0.5f,  0.5f,  0.5f),
    vec3(-0.5f,  0.5f, -0.5f)
};

void main()
{
  frag_position = vertices[gl_VertexIndex];
  gl_Position = vec4(frag_position, 1.0f);
}