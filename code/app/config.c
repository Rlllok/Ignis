#include "base/base_include.h"
#include "os/os_include.h"

#include "base/base_include.c"
#include "os/os_include.c"

// typedef U16 ConfigOptionType;
// enum ConfigOptionTypeEnum
// {
//   ConfigOptionType_None,
//   ConfigOptionType_I32,
//   ConfigOptionType_F32,
//   ConfigOptionType_B32,
// } ConfigOptionTypeEnum;

typedef enum
{
  ConfigOptionType_None,
  ConfigOptionType_I32,
  ConfigOptionType_F32,
  ConfigOptionType_B32,
  ConfigOptionType_Str8,
} ConfigOptionTypeEnum;

typedef enum
{
  ST_INIT,
  ST_SIGN,
  ST_POINT,
  ST_INT,
  ST_FLOAT,
  ST_STR_OR_BOOL,
} ReadState;

typedef struct Config Config;
struct Config
{
  Str8 key;
  Str8 value;
  ConfigOptionTypeEnum type;
  union
  {
    I32 v_i32;
    F32 v_f32;
    B32 v_b32;
    Str8 v_str8;
  };
};


ConfigOptionTypeEnum check_type(Str8 value)
{
  ReadState state = ST_INIT;
  for (int i = 0; i < value.length; i++)
  {
    switch (state)
    {
    case ST_INIT:
      if (value.data[i] == '-')
      {
        state = ST_SIGN;
      }
      else if (isdigit(value.data[i]))
      {
        state = ST_INT;
      }
      else if ((value.data[i] == 't') || (value.data[i] == 'f'))
      {
        state = ST_STR_OR_BOOL;
      }
      else
      {
        return ConfigOptionType_Str8;
      }
      break;

    case ST_SIGN:
      if (!isdigit(value.data[i]))
      {
        return ConfigOptionType_Str8;
      }
      state = ST_INT;
      break;

    case ST_INT:
      if (value.data[i] == '.')
      {
        state = ST_POINT;
      }
      else if ((value.data[i] == 'f') && (i == (value.length - 1)))
      {
        return ConfigOptionType_F32;
      }
      else if (!isdigit(value.data[i]))
      {
        return ConfigOptionType_Str8;
      }
      break;

    case ST_POINT:
      if (!isdigit(value.data[i]))
      {
        return ConfigOptionType_Str8;
      }
      state = ST_FLOAT;
      break;

    case ST_FLOAT:
      if ((value.data[i] == 'f') && (i == (value.length - 1)))
      {
        return ConfigOptionType_F32;
      }
      else if ((value.data[i] == '.') || !isdigit(value.data[i]))
      {
        return ConfigOptionType_Str8;
      }
      break;

    case ST_STR_OR_BOOL:
      if (!isalpha(value.data[i]))
      {
        return ConfigOptionType_Str8;
      }
      break;

    default:
      break;
    }
  }

  switch (state)
  {
  case ST_STR_OR_BOOL:
    if (Str8Equal(value, Str8C("true")) || Str8Equal(value, Str8C("false")))
    {
      return ConfigOptionType_B32;
    }
    return ConfigOptionType_Str8;
  case ST_INT:
    return ConfigOptionType_I32;
  case ST_FLOAT:
    return ConfigOptionType_F32;
  default:
    return ConfigOptionType_Str8;
  }
}


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
        Str8 value = Str8C(temp_str);
        switch(check_type(value))
        {
          case ConfigOptionType_None:
            LOG_INFO("%s - NONE\n", CFromStr8(value));
            break;
          case ConfigOptionType_I32:
            LOG_INFO("%s - INT\n", CFromStr8(value));
            // config_arr[line_n].v_i32 = convert_type(value, ConfigOptionType_I32);
            break;
          case ConfigOptionType_F32:
            LOG_INFO("%s - FLOAT\n", CFromStr8(value));
            break;
          case ConfigOptionType_B32:
            LOG_INFO("%s - BOOL\n", CFromStr8(value));
            break;
          case ConfigOptionType_Str8:
            LOG_INFO("%s - STR\n", CFromStr8(value));
            break;
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
  LOG_INFO("\n");
  for(U16 i = 0; i < line_n; i++)
  {
    LOG_INFO("KEY: `%s`; VALUE: `%s`\n", CFromStr8(config_arr[i].key), CFromStr8(config_arr[i].value));
  }

  fclose(file);
  
  return 0;
}


// ConfigOptionTypeEnum check_type(Str8 value)
// {
//   ConfigOptionTypeEnum type = ConfigOptionType_None;
//   for(int i = 0; i < value.length; i++)
//   {
//     if (value.data[i] == '-')
//     {
//       if((i == 0) && (value.length > 1))
//       {
//         if (isdigit(value.data[1]))
//         {
//           type = ConfigOptionType_I32;
//           continue;
//         }
//       }
//       return ConfigOptionType_Str8;
//     }
//     else if (value.data[i] == '.')
//     {
//       if (type != ConfigOptionType_I32) return ConfigOptionType_Str8;
      
//       type = ConfigOptionType_F32;
//     }
//     else if (isalpha(value.data[i]))
//     {
//       if ((value.data[i] == 'f') && (i == value.length - 1) && ((type == ConfigOptionType_F32) || (type == ConfigOptionType_I32)))
//       {
//         return ConfigOptionType_F32;
//       }
      
//       type = ConfigOptionType_Str8;
//     }
//     else if (isdigit(value.data[i]))
//     {
//       if ((type == ConfigOptionType_I32) || (type == ConfigOptionType_F32))
//       {
//         continue;
//       }
//       if (i == 0)
//       {
//         type = ConfigOptionType_I32;
//       }
//     }
//     else
//     {
//       return ConfigOptionType_Str8;
//     }
//   }
//   if (type == ConfigOptionType_Str8)
//   {
//     if(Str8Equal(value, Str8C("true")) || Str8Equal(value, Str8C("false")))
//     {
//       return ConfigOptionType_B32;
//     }
//   }
//   return type;
// }