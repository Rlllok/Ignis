#version 460

layout(set = 3, binding = 1) uniform sampler2D font_bitmap;

layout(location = 0) in vec2 uv;

layout(location = 0) out vec4 color_target;

void main(void)
{
  color_target = texture(font_bitmap, uv);
}
