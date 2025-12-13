#include "base/base_include.h"
#include "os/os_include.h"

#include "base/base_include.c"
#include "os/os_include.c"

typedef U16 ConfigOptionType;
enum ConfigOptionTypeEnum
{
  ConfigOptionType_None,
  ConfigOptionType_I32,
  ConfigOptionType_F32,
  ConfigOptionType_B32,
} ConfigOptionTypeEnum;

typedef struct Config Config;
struct Config
{
  Str8 key;
  ConfigOptionType type;
  union
  {
    I32 v_i32;
    F32 v_f32;
    B32 v_b32;
  };
};

I32 main()
{
  LOG_INFO("Hello Sashko\n");

  Str8 file_path = Str8C("D:/programming/Ignis/code/app/config_example.ini");
  
  FILE* file = fopen(CFromStr8(file_path), "r");
  if (file == NULL)
  {
    LOG_ERROR("Cannot open file %s\n", CFromStr8(file_path));
    return 0;
  }

  Arena* arena = AllocateArena(Kilobytes(16));
  U16 max_line_len = 255;
  U16 max_config_arr_len = 64;
  Str8 line = AllocateStr8(arena, max_line_len);
  // Str8 temp_str = AllocateStr8(arena, max_line_len);
  Config config_arr[max_config_arr_len];
  U16 line_n = 0;
  U16 line_i;
  U16 key_value_switch;
  char temp_str[255];
   
  while(fgets((char*)line.data, max_line_len, file) != NULL)
  {    
    if(line.data[0] == '[' || line.data[0] == ';' || line.data[0] == '\n' || line.data[0] == '\0') continue;

    line_i = 0;
    key_value_switch = 0;
    for(U16 i = 0; i < max_line_len; i++)
    {
      if(line.data[i] == '\n' || line.data[i] == '\0') {
        temp_str[line_i] = '\0';
        if(key_value_switch != 1) {
          LOG_ERROR("SRAKA 1")
          exit(1);
        }
        config_arr[line_n].value = CopyStr8(arena, Str8C(temp_str));
        break;
      }
      if(line.data[i] == ' ') continue;
      if(line.data[i] == '=')
      {
        temp_str[line_i] = '\0';
        if(key_value_switch != 0) {
          LOG_ERROR("SRAKA 0")
          exit(1);
        }
        config_arr[line_n].key = CopyStr8(arena, Str8C(temp_str));
        key_value_switch++;
        line_i = 0;
        continue;
      }

      temp_str[line_i] = line.data[i];
      line_i++;
    }
    line_n++;
    // U64 eq_sign_pos = GetSymbolPosition(line, '=');
    // U64 eol_sign_pos = GetSymbolPosition(line, '\n');
    // U64 eof_sign_pos = GetSymbolPosition(line, '\0');
    // U64 len_till_eol = Min(eof_sign_pos, eol_sign_pos) - eq_sign_pos;
    
    // config_arr[line_n].key = SubStr8(arena, line, 0, eq_sign_pos);
    // config_arr[line_n].value = SubStr8(arena, line, eq_sign_pos + 1, len_till_eol);
  }
  for(U16 i = 0; i < line_n; i++)
  {
    LOG_INFO("KEY: `%s`; VALUE: `%s`\n", CFromStr8(config_arr[i].key), CFromStr8(config_arr[i].value));
  }

  fclose(file);
  
  return 0;
}
