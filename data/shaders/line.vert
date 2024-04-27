#version 460

layout(location = 0) in vec3 position;

layout(location = 0) out vec3 fragColor;

void main()
{
    fragColor = vec3(0.0f, 1.0f, 0.1f);
    gl_Position = vec4(position, 1.0f);
}
