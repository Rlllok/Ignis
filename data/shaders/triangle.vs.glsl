#version 460
    
layout(location = 0) in vec3 in_position;

layout(location = 0) out vec3 out_color;

void main()
{
    gl_Position = vec4(in_position, 1.0);

    vec3 in_color = vec3(0.7f, 0.2f, 0.4f);
    out_color = in_color;
}
