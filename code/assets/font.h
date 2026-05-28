#pragma once

typedef struct AST_FontGlyph AST_FontGlyph;
struct AST_FontGlyph {
  RHI_Texture texture;
  I32         width;
  I32         height;
  I32         x_offset;
  I32         y_offset;
  I32         advance;
  I32         lsb;
};

typedef struct AST_Font AST_Font;
struct AST_Font {
  AST_FontGlyph glyphs[96]; // from space (32) to ~ (126)
  I32 ascent;
  F32 scale;
  I32 font_size;
};

func AST_Font AST_FontFromTTF(Arena* arena, RHI_CommandBuffer command_buffer, RHI_Buffer transfer_buffer, Str8 file_path, U16 size);

func Vec2F32 AST_TextSize(Str8 text, AST_Font* font);
