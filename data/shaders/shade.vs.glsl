#version 460

vec2 vertecies[4] = {
	{ -1.0f, 1.0f},
	{ 1.0f, 1.0f},
	{ 1.0f, -1.0f},
	{ -1.0f, -1.0f},
};

int indecies[6] = {0,1,3,1,2,3};

layout(location = 0) out out_values {
  vec2 uv;
};

void main() {
  vec2 xy = vertecies[indecies[gl_VertexIndex]];
    
  uv = (xy + 1.0) / 2.0f;

  gl_Position = vec4(xy, 0.0f, 1.0f);
}
