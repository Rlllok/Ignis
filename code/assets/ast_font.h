#pragma once

#include "base/base_file.h"

#define TTFFontTag(a, b, c ,d) (a<<24|b<<16|c<<8|d)
#define TTF_GLYF_TAG TTFFontTag('g', 'l', 'y', 'f')
#define TTF_LOCA_TAG TTFFontTag('l', 'o', 'c', 'a')
#define TTF_HEAD_TAG TTFFontTag('h', 'e', 'a', 'd')
#define TTF_CMAP_TAG TTFFontTag('c', 'm', 'a', 'p')

struct TTFOffsetSubtable
{
  U32 scaler_type;
  U16 num_tables;
  U16 search_range;
  U16 entry_selector;
  U16 range_shift;
};

struct TTFTableDirectory
{
  union
  {
    U32 tag;
    U8 tag_char[4];
  };
  U32 check_sum;
  U32 offset; // offset from the start of the file
  U32 length;
};

struct TTFCmapSubtable
{
  U16 platform_id;
  U16 platform_specific_id;
  U32 offset;
};

struct TTFCmapTable
{
  U16 version;
  U16 num_subtables;
  TTFCmapSubtable* subtables;
};

struct TTFFormat4
{
  U16 format;
  U16 length;
  U16 language;
  U16 seg_count_x2;
  U16 search_range;
  U16 entry_selector;
  U16 range_shift;
  U16 reserved_pad;
  U16* start_code;
  U16* end_code;
  U16* id_delta;
  U16* id_range_offset;
  U16* glyph_id_array;
};

union TTFGlyphFlag
{
  struct
  {
     U8 on_curve: 1;
     U8 x_short: 1;
     U8 y_short: 1;
     U8 repeat: 1;
     U8 x_short_pos: 1;
     U8 y_short_pos: 1;
     U8 reserved1: 1;
     U8 reserved2: 1;
  };
  U8 flag;
};

struct TTFGlyphData
{
  U16 num_contours;
  I16 x_min;
  I16 y_min;
  I16 x_max;
  I16 y_max;
  U16 instruction_length;
  U8* instructions;
  TTFGlyphFlag* flags;
  I16* x_coordinates;
  I16* y_coordinates;
  U16* end_pts_of_contours;
};

struct TTFData
{
  TTFOffsetSubtable offset_subtable;
  TTFTableDirectory* table_directories;
  TTFFormat4* format;
  TTFCmapTable cmap;
  U16 units_per_em;
  U16 loca_type;
  U8* head;
  U8* glyf;
  U8* loca;
};

func TTFOffsetSubtable _AST_TTFReadOffsetSubtable(U8** bytes);
func TTFTableDirectory* _AST_TTFReadTableDirectories(Arena* arena, U8** bytes, U16 num_tables);
func TTFCmapTable _AST_TTFReadCmapTable(Arena* arena, U8** bytes);
func TTFFormat4* _AST_TTFReadFormat4(Arena* arena, U8** bytes);

func I32 _AST_TTFGetGlyphIndex(U16 code, TTFFormat4* format);
func U32 _AST_TTF_GetGlyphOffset(U32 glyph_index, U8* loca, I32 loca_type); // --AlNov: @NOTE Test version

func TTFData AST_GetTTFData(Arena* arena, Str8 ttf_file);
func TTFGlyphData AST_GetTTFGlyphData(Arena* arena, TTFData ttf_data, U32 glyph_index);

struct GlyphData
{
  Vec2f points[1024];
  I32 contur_end_indecies[128];
  
  I32 num_conturs;
  I32 num_points;
};

func GlyphData AST_GetGlyphDataFromTTF(Arena* arena, TTFData ttf_data, I32 unicode);

struct FontData
{
  GlyphData glyphs[256];
};
