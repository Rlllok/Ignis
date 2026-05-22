using namespace metal;

constant float4 vertecies[] = {
  float4(0.0f, 1.0f, 0.0f, 1.0f),
  float4(1.0f, 1.0f, 0.0f, 1.0f),
  float4(1.0f, 0.0f, 0.0f, 1.0f),
  float4(0.0f, 0.0f, 0.0f, 1.0f),
};

constant float2 uv[] = {
  float2(0.0f, 1.0f),
  float2(1.0f, 1.0f),
  float2(1.0f, 0.0f),
  float2(0.0f, 0.0f),
};

constant uint indecies[] = {
  0, 1, 3,
  1, 2, 3
};

struct GlyphData {
  float4x4         projection;
  float4           position_size;
  texture2d<float> texture;
};

struct VertexOutput {
  float4 position [[position]];
  float2 uv;
  uint   instance_index;
};

vertex VertexOutput VertexMain(
  uint vertex_index [[vertex_id]],
  uint instance_index [[instance_id]],
  constant GlyphData* glyph_data [[buffer(1)]]
) {
  VertexOutput output = {0};

  GlyphData glyph = glyph_data[instance_index];

  float4 vertex_position = vertecies[indecies[vertex_index]]*float4(glyph.position_size.z, glyph.position_size.w, 1.0f, 1.0f) + float4(glyph.position_size.x, glyph.position_size.y, 0.0f, 0.0f);
  output.position = glyph.projection*vertex_position;
  output.uv = uv[indecies[vertex_index]];

  return output;
}
