#version 460

layout(location = 0) in vec3 a_position;

layout(set = 0, binding = 0) uniform UData
{
    vec3 color;
    vec2 p0;
    vec2 p1;
    vec2 c0;
    vec2 c1;
} u_data;

layout(location = 0) out struct FSData
{
    vec3 color;
    vec2 p0;
    vec2 p1;
    vec2 c0;
    vec2 c1;
} fs_data;

void main()
{
    fs_data.color = u_data.color;
    fs_data.p0 = u_data.p0;
    fs_data.p1 = u_data.p1;
    fs_data.c0 = u_data.c0;
    fs_data.c1 = u_data.c1;
        
    gl_Position = vec4(a_position.xy, 0.0f, 1.0f);
}
