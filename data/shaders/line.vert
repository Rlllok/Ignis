#version 460

layout(location = 0) in vec3 position;

layout(location = 0) out vec3 fragColor;

void main()
{
    fragColor = vec3(0.3f, 0.7f, 0.6f);
    gl_Position = vec4(position, 1.0f);
}
