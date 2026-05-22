#include "font.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "third_party/stb/stb_truetype.h"

func AST_Font
AST_FontFromTTF(Arena* arena, Str8 file_path, U16 size) {
  AST_Font result = ZeroStruct();

  stbtt_fontinfo font_info;

  FILE* file = fopen(CFromStr8(file_path), "rb");
  Assert(file);

  fseek(file, 0L, SEEK_END);
  U64 file_size = ftell(file);
  rewind(file);

  U8* ttf_content = (U8*)PushArena(arena, file_size);
  fread(ttf_content, file_size, 1, file);
  fclose(file);

  stbtt_InitFont(&font_info, ttf_content, stbtt_GetFontOffsetForIndex(ttf_content, 0));

  for (I32 ascii_code = 33; ascii_code <= 126; ascii_code += 1) {
    F32 scale = stbtt_ScaleForPixelHeight(&font_info, size);

    I32 x0 = 0;
    I32 y0 = 0;
    I32 x1 = 0;
    I32 y1 = 0;
    stbtt_GetCodepointBitmapBox(&font_info, ascii_code, scale, scale, &x0, &y0, &x1, &y1);

    AST_FontGlyph* glyph = result.glyphs + ascii_code - 32;
    glyph->width = x1 - x0;
    glyph->height = y1 - y0;

    glyph->bitmap = (U8*)PushArena(arena, glyph->width*glyph->height);
    stbtt_MakeCodepointBitmap(&font_info, glyph->bitmap, glyph->width, glyph->height, glyph->width, scale, scale, ascii_code);
  }

  result.font_size = size;

  return result;
}
