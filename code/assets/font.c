#include "font.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "third_party/stb/stb_truetype.h"

func AST_Font
AST_FontFromTTF(Arena* arena, RHI_CommandBuffer command_buffer, RHI_Buffer transfer_buffer, Str8 file_path, U16 size) {
  AST_Font result = ZeroStruct();

  stbtt_fontinfo font_info;

  FILE* file = fopen(CFromStr8(file_path), "rb");
  Assert(file);

  fseek(file, 0L, SEEK_END);
  U64 file_size = ftell(file);
  rewind(file);

  ScratchArena scratch = BeginScratchArena(arena); {
    U8* ttf_content = (U8*)PushArena(arena, file_size);
    fread(ttf_content, file_size, 1, file);
    fclose(file);

    stbtt_InitFont(&font_info, ttf_content, stbtt_GetFontOffsetForIndex(ttf_content, 0));
    F32 scale = stbtt_ScaleForPixelHeight(&font_info, size);

    for (I32 ascii_code = 32; ascii_code <= 126; ascii_code += 1) {
      I32 x0 = 0;
      I32 y0 = 0;
      I32 x1 = 0;
      I32 y1 = 0;
      stbtt_GetCodepointBitmapBox(&font_info, ascii_code, scale, scale, &x0, &y0, &x1, &y1);

      AST_FontGlyph* glyph = result.glyphs + ascii_code - 32;
      glyph->x_offset = x0;
      glyph->y_offset = y0;
      glyph->width = x1 - x0;
      glyph->height = y1 - y0;
      stbtt_GetCodepointHMetrics(&font_info, ascii_code, &glyph->advance, &glyph->lsb);

      U8* bitmap = (U8*)PushArena(arena, glyph->width*glyph->height);
      stbtt_MakeCodepointBitmap(&font_info, bitmap, glyph->width, glyph->height, glyph->width, scale, scale, ascii_code);

      if (glyph->width > 0 && glyph->height > 0) {
        result.glyphs[ascii_code - 32].texture = RHI_CreateTexture(&(RHI_TextureCreateInfo) {
          .kind = RHI_TextureKind_2D,
          .format = RHI_TextureFormat_R8_UNORM,
          .usage_flags = RHI_TEXTURE_USAGE_FLAG_SAMPLED|RHI_TEXTURE_USAGE_FLAG_TRANSFER_DST,
          .width = glyph->width,
          .height = glyph->height,
          .depth = 1,
          .num_levels = 1,
        });
        U64 texture_offset = RHI_PushBuffer(transfer_buffer, bitmap, glyph->width*glyph->height);
        RHI_BeginCommandBuffer(command_buffer); {
          RHI_CopyBufferToTexture(command_buffer, transfer_buffer, texture_offset, result.glyphs[ascii_code - 32].texture);
        }
        RHI_EndCommandBuffer(command_buffer);

        RHI_Semaphore semaphore = RHI_CreateSemaphore();
        RHI_SemaphoreSignalInfo signal_semaphore = {
          .semaphore = semaphore,
          .value = 1,
        };
        RHI_SubmitCommandBuffer(command_buffer, 0, 0, &signal_semaphore, 1);
        RHI_WaitSemaphore(semaphore, 1);
        RHI_DestroySemaphore(semaphore);
      }
    }

    result.font_size = size;
    stbtt_GetFontVMetrics(&font_info, &result.ascent,0,0);
    result.scale = scale;
  }
  EndScratchArena(scratch);

  return result;
}

func Vec2F32
AST_TextSize(Str8 text, AST_Font* font) {
  Vec2F32 result = MakeVec2F32(0.0f, 0.0f);
  for (I32 character_index = 0; character_index < text.length; character_index += 1) {
    U8 character = text.data[character_index];
    AST_FontGlyph* glyph = font->glyphs + (character - 32);
    if (character == ' ') {
      result.x += font->glyphs[' ' - 32].width + glyph->advance*font->scale;
    }
    result.x += glyph->width;
    result.y = Max(result.y, glyph->height);
  }
  result.x += (F32)(font->glyphs[text.data[text.length - 1] - 32].advance)*font->scale;
  // result.x /= 2.0f;
  return result;
}
