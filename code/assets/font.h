#pragma once

typedef struct AST_FontGlyph AST_FontGlyph;
struct AST_FontGlyph {
  U8* bitmap;
  U16 width;
  U16 height;
};

typedef struct AST_Font AST_Font;
struct AST_Font {
  AST_FontGlyph glyphs[96]; // from space (32) to ~ (126)
  U16 font_size;
};

func AST_Font AST_FontFromTTF(Arena* arena, Str8 file_path, U16 size);
