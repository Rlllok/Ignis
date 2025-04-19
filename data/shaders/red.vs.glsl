#version 460
    
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;

layout(location = 0) out vec3 out_color;
layout(location = 1) out vec3 out_position;
layout(location = 2) out vec3 out_normal;

void main()
{
    gl_Position = vec4(in_position * vec3(1.0f, -1.0f, 1.0f) * 0.8f, 1.0f);

    vec3 in_color = vec3(0.7f, 0.2f, 0.4f);
    out_color = in_normal;
    out_position = in_position;
    out_normal = in_normal;
}
