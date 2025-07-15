#version 460

layout(location = 0) in vec3 a_position;

layout(set = 0, binding = 0) uniform UData
{
    vec2 mouse_position;
    float time;
    float dt;
} u_data;

layout(location = 0) out struct FSData
{
    vec2 mouse_position;
    float time;
    float dt;
} fs_data;

void main()
{
    fs_data.mouse_position = u_data.mouse_position;
    fs_data.time = u_data.time;
    fs_data.dt = u_data.dt;
        
    gl_Position = vec4(a_position.xy, 0.0f, 1.0f);
}
