#include "ast_font.h"

#include "base/base_file.h"
#include "os/os_memory.h"

func TTFOffsetSubtable
_AST_TTFReadOffsetSubtable(U8** bytes)
{
  TTFOffsetSubtable offset_subtable = {};

  offset_subtable.scaler_type = ReadMoveBytes32BE(*bytes);
  offset_subtable.num_tables = ReadMoveBytes16BE(*bytes);
  offset_subtable.search_range = ReadMoveBytes16BE(*bytes);
  offset_subtable.entry_selector = ReadMoveBytes16BE(*bytes);
  offset_subtable.range_shift = ReadMoveBytes16BE(*bytes);

  return offset_subtable;
}

func TTFTableDirectory*
_AST_TTFReadTableDirectories(Arena* arena, U8** bytes, U16 num_tables)
{
  TTFTableDirectory* directories = (TTFTableDirectory*)PushArena(arena, sizeof(TTFTableDirectory)*num_tables);

  for (I32 i = 0; i < num_tables; i += 1)
  {
    TTFTableDirectory* directory = directories + i;
    directory->tag = ReadMoveBytes32BE(*bytes);
    directory->check_sum = ReadMoveBytes32BE(*bytes);
    directory->offset = ReadMoveBytes32BE(*bytes);
    directory->length = ReadMoveBytes32BE(*bytes);
  }

  return directories;
}

func TTFCmapTable
_AST_TTFReadCmapTable(Arena* arena, U8** bytes)
{
  TTFCmapTable cmap_table = {};

  cmap_table.version = ReadMoveBytes16BE(*bytes);
  cmap_table.num_subtables = ReadMoveBytes16BE(*bytes);
  cmap_table.subtables = (TTFCmapSubtable*)PushArena(arena, sizeof(TTFCmapSubtable)*cmap_table.num_subtables);
  for (I32 i = 0; i < cmap_table.num_subtables; i += 1)
  {
    TTFCmapSubtable* subtable = cmap_table.subtables + i;
    subtable->platform_id = ReadMoveBytes16BE(*bytes);
    subtable->platform_specific_id = ReadMoveBytes16BE(*bytes);
    subtable->offset = ReadMoveBytes32BE(*bytes);
  }

  return cmap_table;
}

func TTFFormat4*
_AST_TTFReadFormat4(Arena* arena, U8** bytes)
{
  U16 length = ReadBytes16BE(*bytes + 2);
  U8* starting_ptr = *bytes;
  TTFFormat4* format = (TTFFormat4*)PushArena(arena, length + sizeof(U16)*5);

  format->format = ReadMoveBytes16BE(*bytes);
  format->length = ReadMoveBytes16BE(*bytes);
  format->language = ReadMoveBytes16BE(*bytes);
  format->seg_count_x2 = ReadMoveBytes16BE(*bytes);
  format->search_range = ReadMoveBytes16BE(*bytes);
  format->entry_selector = ReadMoveBytes16BE(*bytes);
  format->range_shift = ReadMoveBytes16BE(*bytes);

  format->end_code = (U16*)((U8*)format + sizeof(TTFFormat4));
  format->start_code = format->end_code + format->seg_count_x2/2;
  format->id_delta = format->start_code + format->seg_count_x2/2;
  format->id_range_offset = format->id_delta + format->seg_count_x2/2;
  format->glyph_id_array = format->id_range_offset + format->seg_count_x2/2;

  U8* start_code_ptr = *bytes + (format->seg_count_x2 + 2);
  U8* id_delta_ptr = *bytes + (format->seg_count_x2*2 + 2);
  U8* id_range_ptr = *bytes + (format->seg_count_x2*3 + 2);

  for (I32 i = 0; i < format->seg_count_x2/2; i += 1)
  {
    format->end_code[i] = ReadBytes16BE(*bytes + i*2);
    format->start_code[i] = ReadBytes16BE(start_code_ptr + i*2);
    format->id_delta[i] = ReadBytes16BE(id_delta_ptr + i*2);
    format->id_range_offset[i]= ReadBytes16BE(id_range_ptr + i*2);
  }

  *bytes += (format->seg_count_x2*4 + 2);

  I32 remaining_bytes = format->length - (*bytes - starting_ptr);

  for (I32 i = 0; i < remaining_bytes/2; i += 1)
  {
    format->glyph_id_array[i] = ReadMoveBytes16BE(*bytes);
  }

  return format;
}

func I32
_AST_TTFGetGlyphIndex(U16 code, TTFFormat4* format)
{
  I32 index = -1;

  for (I32 i = 0; i < format->seg_count_x2/2; i += 1)
  {
    if (format->end_code[i] > code)
    {
      index = i;
      break;
    }
  }

  if (index == -1) return 0;

  if (format->start_code[index] < code)
  {
    if (format->id_range_offset[index] != 0)
    {
      U16* ptr = format->id_range_offset + index + format->id_range_offset[index]/2;
      ptr += code - format->start_code[index];
      
      if (*ptr == 0) return 0;

      return (*ptr + format->id_delta[index])%(U16_MAX+1);
    }
    else
    {
      return (code + format->id_delta[index])%(U16_MAX+1);
    }
  }

  return 0;
}

func U32
_AST_TTF_GetGlyphOffset(U32 glyph_index, U8* loca, I32 loca_type)
{
  U32 offset = 0;
  
  if (loca_type == 1)
  {
    offset = ReadBytes32BE((U32*)loca + glyph_index);
  }
  else
  {
    offset = ReadBytes16BE((U16*)loca + glyph_index)*2;
  }

  return offset;
}

func TTFData
AST_GetTTFData(Arena* arena, Str8 ttf_file)
{
  TTFData ttf_data = {};
  
  FileHandle file = FileOpen(ttf_file);
  FileData file_data = ReadFileBinary(arena, file);
  FileClose(&file);

  ttf_data.offset_subtable = _AST_TTFReadOffsetSubtable(&file_data.pointer);
  ttf_data.table_directories = _AST_TTFReadTableDirectories(arena, &file_data.pointer, ttf_data.offset_subtable.num_tables);

  // --AlNov: @DEBUG
  LOG_INFO("#)\ttag\tlen\toffset");
  for (I32 i = 0; i < ttf_data.offset_subtable.num_tables; i += 1)
  {
    TTFTableDirectory* table = ttf_data.table_directories + i;
    LOG_INFO("%d)\t%c%c%c%c\t%d\t%d", i+1, table->tag_char[3], table->tag_char[2], table->tag_char[1], table->tag_char[0], table->length, table->offset);
  }

  for (I32 i = 0; i < ttf_data.offset_subtable.num_tables; i += 1)
  {
    switch(ttf_data.table_directories[i].tag)
    {
      case TTF_CMAP_TAG:
      {
        U8* cmap_data_ptr = file_data.data + ttf_data.table_directories[i].offset;
        ttf_data.cmap = _AST_TTFReadCmapTable(arena, &cmap_data_ptr);
        // --AlNov: @DEBUG
        LOG_INFO("CMAP Data:");
        for (I32 j = 0; j < ttf_data.cmap.num_subtables; j += 1)
        {
          TTFCmapSubtable* st = ttf_data.cmap.subtables + j;
          LOG_INFO("%d)\t%d\t%d\t%d", j+1, st->platform_id, st->platform_specific_id, st->offset);
        }
        
        U8* format_data_ptr = file_data.data + (ttf_data.table_directories[i].offset + ttf_data.cmap.subtables[0].offset);
        ttf_data.format = _AST_TTFReadFormat4(arena, &format_data_ptr);
        
        LOG_INFO("Format: %d, Length: %d, Language: %d, Segment Count: %d",
                 ttf_data.format->format, ttf_data.format->length, ttf_data.format->language, ttf_data.format->seg_count_x2/2);

        LOG_INFO("Segment Ranges:\tstartCode\tendCode\tidDelta\tidRangeOffset");
        for (I32 i = 0; i < 8; i += 1)
        {
      		LOG_INFO("--------------:\t% 9d\t% 7d\t% 7d\t% 12d", ttf_data.format->start_code[i], ttf_data.format->end_code[i], ttf_data.format->id_delta[i], ttf_data.format->id_range_offset[i]);
        }
      } break;
      case TTF_HEAD_TAG:
      {
        // --AlNov: @NOTE It should be copied
        ttf_data.head = file_data.data + ttf_data.table_directories[i].offset;
        ttf_data.units_per_em = ReadBytes16BE(ttf_data.head + 18);
        ttf_data.loca_type = ReadBytes16BE(ttf_data.head + 50);
      } break;
      case TTF_GLYF_TAG:
      {
        ttf_data.glyf = file_data.data + ttf_data.table_directories[i].offset;
      } break;
      case TTF_LOCA_TAG:
      {
        ttf_data.loca = file_data.data + ttf_data.table_directories[i].offset;
      } break;

      default: break;
    }
  }

  return ttf_data;
}

func TTFGlyphData
AST_GetTTFGlyphData(Arena* arena, TTFData ttf_data, U32 glyph_index)
{
  TTFGlyphData glyph_data = {};
  
  U32 glyph_offset = _AST_TTF_GetGlyphOffset(glyph_index, ttf_data.loca, ttf_data.loca_type);
  U8* glyph_ptr = ttf_data.glyf + glyph_offset;
  
  {
    glyph_data.num_contours = ReadMoveBytes16BE(glyph_ptr);
    glyph_data.x_min = ReadMoveBytes16BE(glyph_ptr);
    glyph_data.y_min = ReadMoveBytes16BE(glyph_ptr);
    glyph_data.x_max = ReadMoveBytes16BE(glyph_ptr);
    glyph_data.y_max = ReadMoveBytes16BE(glyph_ptr);

    glyph_data.end_pts_of_contours = (U16*)PushArena(arena, glyph_data.num_contours*sizeof(U16));
    for (I32 i = 0; i < glyph_data.num_contours; i += 1)
    {
      glyph_data.end_pts_of_contours[i] = ReadMoveBytes16BE(glyph_ptr);
    }

    glyph_data.instruction_length = ReadMoveBytes16BE(glyph_ptr);
    glyph_data.instructions = (U8*)PushArena(arena, glyph_data.instruction_length);
    memcpy(glyph_data.instructions, glyph_ptr, glyph_data.instruction_length);
    glyph_ptr += glyph_data.instruction_length;

    I32 last_index = glyph_data.end_pts_of_contours[glyph_data.num_contours - 1];
    glyph_data.flags = (TTFGlyphFlag*)PushArena(arena, last_index + 1);

    for (I32 i = 0; i < (last_index + 1); i += 1)
    {
      glyph_data.flags[i].flag = *glyph_ptr;
      glyph_ptr += 1;

      if (glyph_data.flags[i].repeat)
      {
        U8 repeat_count = *glyph_ptr;
        while (repeat_count > 0)
        {
          i += 1;
          glyph_data.flags[i].flag = glyph_data.flags[i - 1].flag;
          repeat_count -= 1;
        }
        glyph_ptr += 1;
      }
    }

    glyph_data.x_coordinates = (I16*)PushArena(arena, (last_index + 1)*2);
    I16 prev_coordinate = 0;
    I16 current_coordinate = 0;

    for (I32 i = 0; i < (last_index + 1); i += 1)
    {
      I32 flag_combined = glyph_data.flags[i].x_short << 1 | glyph_data.flags[i].x_short_pos;
      switch (flag_combined)
      {
        case 0:
        {
          current_coordinate = ReadMoveBytes16BE(glyph_ptr);
        } break;
        case 1:
        {
          current_coordinate = 0;
        } break;
        case 2:
        {
          current_coordinate = (*(U8*)(glyph_ptr)*-1);
          glyph_ptr += 1;
        } break;
        case 3:
        {
          current_coordinate = (*(U8*)(glyph_ptr));
          glyph_ptr += 1;
        } break;
      }

      glyph_data.x_coordinates[i] = current_coordinate + prev_coordinate;
      prev_coordinate = glyph_data.x_coordinates[i];
    }

    glyph_data.y_coordinates = (I16*)OS_AllocateMemory((last_index + 1)*2);
    prev_coordinate = 0;
    current_coordinate = 0;

    for (I32 i = 0; i < (last_index + 1); i += 1)
    {
      I32 flag_combined = glyph_data.flags[i].y_short << 1 | glyph_data.flags[i].y_short_pos;
      switch (flag_combined)
      {
        case 0:
        {
          current_coordinate = ReadMoveBytes16BE(glyph_ptr);
        } break;
        case 1:
        {
          current_coordinate = 0;
        } break;
        case 2:
        {
          current_coordinate = (*(U8*)(glyph_ptr)*-1);
          glyph_ptr += 1;
        } break;
        case 3:
        {
          current_coordinate = (*(U8*)(glyph_ptr));
          glyph_ptr += 1;
        } break;
      }

      glyph_data.y_coordinates[i] = current_coordinate + prev_coordinate;
      prev_coordinate = glyph_data.y_coordinates[i];
    }

  	LOG_INFO("#contours\t(xMin,yMin)\t(xMax,yMax)\tinst_length");
  	LOG_INFO("%9d\t(%d,%d)\t\t(%d,%d)\t%d", glyph_data.num_contours,
  			glyph_data.x_min, glyph_data.y_min,
  			glyph_data.x_max, glyph_data.y_max,
  			glyph_data.instruction_length);

  	LOG_INFO("#)\t(  x  ,  y  )");
  	int li = glyph_data.end_pts_of_contours[glyph_data.num_contours-1];
  	for(int i = 0; i <= li; ++i) {
  		LOG_INFO("%d)\t(%5f,%5f)", i, (F32)glyph_data.x_coordinates[i]/(F32)ttf_data.units_per_em, (F32)glyph_data.y_coordinates[i]/(F32)ttf_data.units_per_em);
  	}
  }
  
  return glyph_data;
}

func GlyphData AST_GetGlyphDataFromTTF(Arena* arena, TTFData ttf_data, I32 unicode)
{
  I32 ttf_glyph_index = _AST_TTFGetGlyphIndex(unicode, ttf_data.format);
  TTFGlyphData ttf_glyph_data = AST_GetTTFGlyphData(arena, ttf_data, ttf_glyph_index);
  GlyphData glyph_data = {};
  glyph_data.num_conturs = ttf_glyph_data.num_contours;
  
  I32 j = 0;
  for (I32 i = 0; i < ttf_glyph_data.num_contours; i += 1)
  {
    I32 contur_start_index = j;
    for (;j <= ttf_glyph_data.end_pts_of_contours[i]; j += 1)
    {
      I32 counter_length = ttf_glyph_data.end_pts_of_contours[i] - contur_start_index + 1;
      I32 current_index = j;
      I32 next_index = (j+1 - contur_start_index)%counter_length + contur_start_index;

      TTFGlyphFlag flag = *(ttf_glyph_data.flags + j);
      Vec2f point = MakeVec2f(ttf_glyph_data.x_coordinates[current_index], ttf_glyph_data.y_coordinates[current_index]);

      if (flag.on_curve)
      {
        glyph_data.points[glyph_data.num_points] = point/ttf_data.units_per_em;
        glyph_data.num_points += 1;
        
        if (ttf_glyph_data.flags[next_index].on_curve)
        {
          Vec2f next_point = MakeVec2f(ttf_glyph_data.x_coordinates[next_index], ttf_glyph_data.y_coordinates[next_index]);
          Vec2f generated_point = {};
          generated_point.x = point.x + (next_point.x - point.x)*0.5f;
          generated_point.y = point.y + (next_point.y - point.y)*0.5f;
                           
          glyph_data.points[glyph_data.num_points] = generated_point/ttf_data.units_per_em;
          glyph_data.num_points += 1;
        }
      }
      else
      {
        glyph_data.points[glyph_data.num_points] = point/ttf_data.units_per_em;
        glyph_data.num_points += 1;

        if (!(ttf_glyph_data.flags[next_index]).on_curve)
        {
          Vec2f next_point = MakeVec2f(ttf_glyph_data.x_coordinates[next_index], ttf_glyph_data.y_coordinates[next_index]);
          Vec2f generated_point = {};
          generated_point.x = point.x + (next_point.x - point.x)*0.5f;
          generated_point.y = point.y + (next_point.y - point.y)*0.5f;
                           
          glyph_data.points[glyph_data.num_points] = generated_point/ttf_data.units_per_em;
          glyph_data.num_points += 1;
        }
      }
    }

    Vec2f point = MakeVec2f(ttf_glyph_data.x_coordinates[contur_start_index], ttf_glyph_data.y_coordinates[contur_start_index]);
    glyph_data.points[glyph_data.num_points] = point/ttf_data.units_per_em;
    glyph_data.num_points += 1;

    glyph_data.contur_end_indecies[i] = glyph_data.num_points - 1;
  }
  
  return glyph_data;
}
