#version 460
    
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;

layout(set = 0, binding = 0) uniform UData
{
    mat4 projection;
    mat4 view;
    // vec3 color;
} u_data;

layout(location = 0) out vec3 out_color;
layout(location = 1) out vec3 out_position;
layout(location = 2) out vec3 out_normal;

void main()
{
    out_color = vec3(1.0f, 0.0f, 0.0f); //u_data.color;
    out_position = (in_position) + vec3(0.0f, 0.0f, -10.0f);
    out_normal = in_normal;

    gl_Position = u_data.projection * u_data.view * vec4(out_position, 1.0f);
}
