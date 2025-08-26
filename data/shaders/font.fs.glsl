#version 460

layout(set = 2, binding = 0) uniform sampler2D font_bitmap;

layout(location = 0) in vec2 uv;
layout(location = 1) flat in vec4 color;

layout(location = 0) out vec4 color_target;

void main(void)
{
  color_target = color*texture(font_bitmap, uv);
}
