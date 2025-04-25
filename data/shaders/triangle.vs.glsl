#version 460
    
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;

layout(set = 0, binding = 0) uniform UData
{
    mat4 projection;
    mat4 view;
    mat4 model;
} u_data;

layout(location = 0) out vec3 out_color;
layout(location = 1) out vec3 out_position;
layout(location = 2) out vec3 out_normal;

void main()
{
    out_color = vec3(0.7f, 0.4f, 0.3f);
    out_position = vec3(u_data.model * vec4(in_position, 1.0f));
    out_normal = mat3(transpose(inverse(u_data.model))) * in_normal;

    gl_Position = u_data.projection * u_data.view * vec4(out_position, 1.0f);
}
