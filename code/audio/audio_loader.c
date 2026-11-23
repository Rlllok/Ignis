#include "audio_loader.h"

#define AU_GetWaveFileChunkIdCode(c0, c1, c2, c3) (((U32)(c3) << 24)|((U32)(c2) << 16)|((U32)(c1) << 8) | (U32)(c0))

enum {
  AU_WaveFile_ChunkId_Riff = AU_GetWaveFileChunkIdCode('R', 'I', 'F', 'F'),
  AU_WaveFile_ChunkId_Wave = AU_GetWaveFileChunkIdCode('W', 'A', 'V', 'E'),
  AU_WaveFile_ChunkId_Fmt = AU_GetWaveFileChunkIdCode('f', 'm', 't', ' '),
  AU_WaveFile_ChunkId_Fact = AU_GetWaveFileChunkIdCode('f', 'a', 'c', 't'),
  AU_WaveFile_ChunkId_Data = AU_GetWaveFileChunkIdCode('d', 'a', 't', 'a'),
};

#pragma pack(push, 1)
typedef struct AU_WaveFileChunkHeader AU_WaveFileChunkHeader;
struct AU_WaveFileChunkHeader {
  U32 id;
  U32 size;
};

typedef struct AU_WaveFileHeader AU_WaveFileHeader;
struct AU_WaveFileHeader {
  AU_WaveFileChunkHeader riff_chunk;

  U32 wave_id;
};

typedef struct AU_WaveFileFmt AU_WaveFileFmt;
struct AU_WaveFileFmt {
  AU_WaveFileChunkHeader header;

  U16 format_tag;
  U16 channels_number;
  U32 samples_per_sec;
  U32 avg_bytes_per_sec;
  U16 block_align;
  U16 bits_per_sample;
  U16 extension_size;
  U16 valid_bits_per_sample;
  U32 channel_mask;
  U8  subformat[16];
};

typedef struct AU_WaveFileData AU_WaveFileData;
struct AU_WaveFileData {
  AU_WaveFileChunkHeader header;
};
#pragma pack(pop)

typedef struct AU_WaveFileCursor AU_WaveFileCursor;
struct AU_WaveFileCursor {
  AU_WaveFileChunkHeader* current_chunk;
  U8*                     at;
  U8*                     end;
};

func AU_WaveFileCursor
AU_WaveFileInitCursor(U8* first_byte) {
  AU_WaveFileHeader* header = (AU_WaveFileHeader*)first_byte;
  Assert(header->riff_chunk.id == AU_WaveFile_ChunkId_Riff);
  Assert(header->wave_id == AU_WaveFile_ChunkId_Wave);
  
  AU_WaveFileCursor result = {
    .current_chunk = (AU_WaveFileChunkHeader*)(first_byte + sizeof(AU_WaveFileHeader)),
    .at = first_byte + sizeof(AU_WaveFileHeader),
    .end = first_byte + sizeof(AU_WaveFileChunkHeader) + header->riff_chunk.size,
  };
  return result;
}

func B32
AU_WaveFileCursorIsValid(AU_WaveFileCursor cursor) {
  return cursor.at < cursor.end;
}

func AU_WaveFileCursor
AU_WaveFileNextChunk(AU_WaveFileCursor cursor) {
  U64 padding = cursor.current_chunk->size & 1;
  AU_WaveFileCursor result = {
    .current_chunk = (AU_WaveFileChunkHeader*)(cursor.at + sizeof(AU_WaveFileChunkHeader) + cursor.current_chunk->size + padding),
    .at = cursor.at + sizeof(AU_WaveFileChunkHeader) + cursor.current_chunk->size + padding,
    .end = cursor.end,
  };
  return result;
}

func AU_WaveFile
AU_LoadWaveFile(Arena* arena, Str8 path) {
  AU_WaveFile result = ZeroStruct();

  FILE* file = fopen(CFromStr8(path), "rb");
  if (file == 0) {
    LogDebug("Cannot open '%s'.\n", CFromStr8(path));
    return result;
  }

  fseek(file, 0, SEEK_END);
  U64 content_size = ftell(file);
  rewind(file);
  U8* content = (U8*)PushArena(arena, content_size);
  fread(content, content_size, 1, file);

  U32 channels_count = 0;
  U64 samples_size = 0;
  I16* samples = 0;
  for (AU_WaveFileCursor cursor = AU_WaveFileInitCursor(content); AU_WaveFileCursorIsValid(cursor); cursor = AU_WaveFileNextChunk(cursor)) {
    switch(cursor.current_chunk->id) {
      default: {
        U8* id = (U8*)cursor.current_chunk;
        LogDebug("Unknown chunk ID: '%c%c%c%c'\n", id[0], id[1], id[2], id[3]);
      } break;

      case AU_WaveFile_ChunkId_Fmt: {
        AU_WaveFileFmt* fmt = (AU_WaveFileFmt*)cursor.current_chunk;
        Assert(fmt->format_tag == 1);
        Assert(fmt->bits_per_sample == 16);
        Assert(fmt->block_align == (2*fmt->channels_number));

        channels_count = fmt->channels_number;
      } break;

      case AU_WaveFile_ChunkId_Data: {
        AU_WaveFileData* data = (AU_WaveFileData*)cursor.current_chunk;
        samples_size = data->header.size;
        samples = (I16*)((AU_WaveFileChunkHeader*)data + 1);
      } break;
    }
  }

  Assert(channels_count && samples);

  result.channels_count = channels_count;
  result.samples_count = samples_size / (channels_count*sizeof(I16));

  if (channels_count == 1) {
    result.samples[0] = (F32*)PushArena(arena, result.samples_count*sizeof(F32));
    for (I32 sample_index = 0; sample_index < result.samples_count / 2; sample_index += 1) {
      result.samples[0][sample_index] = (F32)samples[sample_index] / I16_MAX;
    }
  }
  else {
    result.samples[0] = (F32*)PushArena(arena, result.samples_count*sizeof(F32));
    result.samples[1] = (F32*)PushArena(arena, result.samples_count*sizeof(F32));
    for (I32 pair_index = 0; pair_index < result.samples_count; pair_index += 1) {
      result.samples[0][pair_index] = (F32)samples[pair_index*2] / I16_MAX;
      result.samples[1][pair_index] = (F32)samples[pair_index*2 + 1] / I16_MAX;
    }
  }

  fclose(file);
  return result;
}
