#include "base/base_include.h"
#include "os/os_include.h"

#include "base/base_include.c"
#include "os/os_include.c"

#include <errno.h>
#include <limits.h>

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

const char* dtype_names[] = {"NONE", "INT", "FLOAT", "BOOL", "STR"};

typedef enum
{
  ST_INIT,
  ST_SIGN,
  ST_POINT,
  ST_INT,
  ST_FLOAT,
} ReadState;

typedef struct ConfigOption ConfigOption;
struct ConfigOption
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
  union
  {
    I32 default_v_i32;
    F32 default_v_f32;
    B32 default_v_b32;
    Str8 default_v_str8;
  };
};

typedef struct Config Config;
struct Config
{
  U16 length;
  ConfigOption* option;
};

ConfigOptionTypeEnum get_type(Str8 value);
U16 find_option_pos(Str8 key_to_find, Config config);
I32 get_config_value_int(Str8 key_to_get, Config config);
F32 get_config_value_float(Str8 key_to_get, Config config);
B32 get_config_value_bool(Str8 key_to_get, Config config);
Str8 get_config_value_str8(Str8 key_to_get, Config config);

B32 fill_config(Arena* arena, Config config, Str8 file_path);

Config create_config(Arena* arena, Str8* config_keys, ConfigOptionTypeEnum* config_types,
                     U32 config_length);

B32 assign_value(ConfigOption* option_ref, Arena* arena, Str8 value);

void print_config(Config config);

I32 main()
{
  Arena* arena = AllocateArena(Kilobytes(16), Kilobytes(16));

  Str8 file_path = Str8C("D:/programming/Ignis/code/app/config_example.ini");
  Str8 config_keys[] = {
    Str8C("app.settings.variable_bool_0"),
    Str8C("app.settings.variable_bool_1"),
    Str8C("app.settings.variable_int"),
    Str8C("app.settings.variable_float"),
    Str8C("app.settings.window.width"),
    Str8C("app.settings.window.height"),
    Str8C("app.settings.window.fullscreen"),
    Str8C("app.program1.enabled"),
    Str8C("app.program1.ratio"),
    Str8C("app.program1.threshold"),
    Str8C("network.proxy.enabled"),
    Str8C("network.proxy.host"),
    Str8C("network.proxy.port"),
    Str8C("random1"),
    Str8C("random2"),
    Str8C("random3"),
    Str8C("random4"),
    Str8C("random5"),
    Str8C("random6"),
    Str8C("random7"),
  };
  ConfigOptionTypeEnum config_types[] = {
    ConfigOptionType_I32,  ConfigOptionType_B32,  ConfigOptionType_I32,  ConfigOptionType_F32,
    ConfigOptionType_I32,  ConfigOptionType_Str8, ConfigOptionType_B32,  ConfigOptionType_I32,
    ConfigOptionType_F32,  ConfigOptionType_F32,  ConfigOptionType_B32,  ConfigOptionType_Str8,
    ConfigOptionType_I32,  ConfigOptionType_None, ConfigOptionType_None, ConfigOptionType_F32,
    ConfigOptionType_Str8, ConfigOptionType_None, ConfigOptionType_None, ConfigOptionType_None,
  };
  U32 config_length = CountArrayElements(config_types);
  printf("%d\n", (int)sizeof(ConfigOption));

  Config config = create_config(arena, config_keys, config_types, config_length);

  B32 success = fill_config(arena, config, file_path);

  LOG_INFO("success: %d\n", (U8)success);
  print_config(config);

  LOG_INFO("\n\n");
  Str8 key = Str8C("app.settings.variable_int");
  LOG_INFO("KEY:  %-35s | VALUE: %-10d\n", CFromStr8(key), get_config_value_int(key, config));
  key = Str8C("app.settings.window.width");
  LOG_INFO("KEY:  %-35s | VALUE: %-10d\n", CFromStr8(key), get_config_value_int(key, config));

  key = Str8C("app.program1.ratio");
  LOG_INFO("KEY:  %-35s | VALUE: %-10f\n", CFromStr8(key), get_config_value_float(key, config));
  key = Str8C("random3");
  LOG_INFO("KEY:  %-35s | VALUE: %-10f\n", CFromStr8(key), get_config_value_float(key, config));

  key = Str8C("app.settings.variable_bool_0");
  LOG_INFO("KEY:  %-35s | VALUE: %-10d\n", CFromStr8(key), get_config_value_bool(key, config));
  key = Str8C("network.proxy.enabled");
  LOG_INFO("KEY:  %-35s | VALUE: %-10d\n", CFromStr8(key), get_config_value_bool(key, config));
  key = Str8C("app.settings.variable_bool_1");
  LOG_INFO("KEY:  %-35s | VALUE: %-10d\n", CFromStr8(key), get_config_value_bool(key, config));

  key = Str8C("network.proxy.host");
  LOG_INFO("KEY:  %-35s | VALUE: %-10s\n", CFromStr8(key),
           CFromStr8(get_config_value_str8(key, config)));
  key = Str8C("random4");
  LOG_INFO("KEY:  %-35s | VALUE: %-10s\n", CFromStr8(key),
           CFromStr8(get_config_value_str8(key, config)));

  return 0;
}

Config create_config(Arena* arena, Str8* config_keys, ConfigOptionTypeEnum* config_types,
                     U32 config_length)
{
  ConfigOption* config_arr = (ConfigOption*)PushArena(arena, sizeof(ConfigOption) * config_length);
  Config config = {
    .length = config_length,
    .option = config_arr,
  };
  return config;
}

B32 fill_config(Arena* arena, Config config, Str8 file_path)
{
  FILE* file = fopen(CFromStr8(file_path), "r");

  U16 max_line_len = 255;

  Str8 line = AllocateStr8(arena, max_line_len);

  if (file == NULL)
  {
    LOG_ERROR("Cannot open file %s\n", CFromStr8(file_path));
    config.length = 0;
    return 0;
  }

  U16 option_i = 0;
  U16 line_i = 0;
  U16 key_value_switch;
  char temp_str[255];

  while (fgets((char*)line.data, max_line_len, file) != NULL)
  {
    if (line.data[0] == '[' || line.data[0] == ';' || line.data[0] == '\n' || line.data[0] == '\0')
      continue;

    line_i = 0;
    key_value_switch = 0;
    for (U16 i = 0; i < max_line_len; i++)
    {
      if (line.data[i] == '\n' || line.data[i] == '\0')
      {
        temp_str[line_i] = '\0';

        if (key_value_switch != 1)
        {
          LOG_ERROR("Bad config. Expected single key value pair on line");
          return 0;
        }
        Str8 value = Str8C(temp_str);
        config.option[option_i].value = CopyStr8(arena, value);  // for debug

        config.option[option_i].type = get_type(value);
        if (!assign_value(&config.option[option_i], arena, value))
        {
          LOG_ERROR("Invalid value '%s' for key '%s'\n", CFromStr8(value),
                    CFromStr8(config.option[option_i].key));
          // return 0;
        }
        break;
      }
      if (line.data[i] == ' ')
        continue;
      if (line.data[i] == '=')
      {
        temp_str[line_i] = '\0';
        if (key_value_switch != 0)
        {
          LOG_ERROR("Bad config. Expected single key value pair on line");
          return 0;
        }
        config.option[option_i].key = CopyStr8(arena, Str8C(temp_str));
        key_value_switch++;
        line_i = 0;
        continue;
      }

      temp_str[line_i] = line.data[i];
      line_i++;
    }
    option_i++;
  }

  if (option_i != config.length)
  {
    return 0;
  }

  fclose(file);
  return 1;
}

I32 get_config_value_int(Str8 key_to_get, Config config)
{
  U16 option_pos = find_option_pos(key_to_get, config);
  if (option_pos >= config.length)
  {
    return config.option[option_pos].default_v_i32;
  }
  return config.option[option_pos].v_i32;
}

F32 get_config_value_float(Str8 key_to_get, Config config)
{
  U16 option_pos = find_option_pos(key_to_get, config);
  if (option_pos >= config.length)
  {
    return config.option[option_pos].default_v_f32;
  }
  return config.option[option_pos].v_f32;
}

B32 get_config_value_bool(Str8 key_to_get, Config config)
{
  U16 option_pos = find_option_pos(key_to_get, config);
  if (option_pos >= config.length)
  {
    return config.option[option_pos].default_v_b32;
  }
  return config.option[option_pos].v_b32;
}

Str8 get_config_value_str8(Str8 key_to_get, Config config)
{
  U16 option_pos = find_option_pos(key_to_get, config);
  if (option_pos >= config.length)
  {
    return config.option[option_pos].default_v_str8;
  }
  return config.option[option_pos].v_str8;
}

U16 find_option_pos(Str8 key_to_find, Config config)
{
  for (U16 i = 0; i < config.length; i++)
  {
    if (Str8Equal(key_to_find, config.option[i].key))
    {
      return i;
    }
  }
  LOG_ERROR("No such key in config");
  return config.length;
}

B32 assign_value(ConfigOption* option_ref, Arena* arena, Str8 value)
{
  switch (option_ref->type)
  {
    case ConfigOptionType_None:
      option_ref->v_str8 = CopyStr8(arena, value);
      return 1;
    case ConfigOptionType_I32:
    {
      errno = 0;
      char* end = NULL;
      const char* temp_str = CFromStr8(value);
      long v = strtol(temp_str, &end, 10);

      if ((end == temp_str)                // no digits
          || (*end != '\0')                // trailing garbage
          || (errno == ERANGE)             // overflow
          || (v < INT_MIN || v > INT_MAX)  // out of range
      )
      {
        return 0;
      }

      option_ref->v_i32 = (I32)v;
      return 1;
    }
    case ConfigOptionType_F32:
    {
      errno = 0;
      char* end = NULL;
      const char* temp_str;
      // remove 'f' symbol in the end if present
      if (value.data[value.length - 1] == 'f')
      {
        temp_str = CFromStr8(SubStr8(arena, value, 0, value.length - 1));
      }
      else
      {
        temp_str = CFromStr8(value);
      }
      float v = strtof(temp_str, &end);

      if ((end == temp_str) || (*end != '\0') || (errno == ERANGE))
      {
        return 0;
      }
      option_ref->v_f32 = v;
      return 1;
    }
    case ConfigOptionType_B32:
      if (Str8Equal(value, Str8C("true")))
      {
        option_ref->v_b32 = 1;
        return 1;
      }
      if (Str8Equal(value, Str8C("false")))
      {
        option_ref->v_b32 = 0;
        return 1;
      }
      return 0;
    case ConfigOptionType_Str8:
      if (value.length < 2)
        return 0;
      option_ref->v_str8 = SubStr8(arena, value, 1, value.length - 2);
      return 1;
  }
  return 0;
}

ConfigOptionTypeEnum get_type(Str8 value)
{
  if (Str8Equal(value, Str8C("true")) || Str8Equal(value, Str8C("false")))
  {
    return ConfigOptionType_B32;
  }
  ReadState state = ST_INIT;
  for (U16 i = 0; i < value.length; i++)
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
        else if ((value.data[i] == '"' && value.data[value.length - 1] == '"') ||
                 (value.data[i] == '\'' && value.data[value.length - 1] == '\''))
        {
          return ConfigOptionType_Str8;
        }
        else
        {
          return ConfigOptionType_None;
        }
        break;

      case ST_SIGN:
        if (!isdigit(value.data[i]))
        {
          return ConfigOptionType_None;
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
          return ConfigOptionType_None;
        }
        break;

      case ST_POINT:
        if (!isdigit(value.data[i]))
        {
          return ConfigOptionType_None;
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
          return ConfigOptionType_None;
        }
        break;

      default:
        break;
    }
  }

  switch (state)
  {
    case ST_INT:
      return ConfigOptionType_I32;
    case ST_FLOAT:
      return ConfigOptionType_F32;
    default:
      return ConfigOptionType_None;
  }
}

void print_config(Config config)
{
  for (U16 i = 0; i < config.length; i++)
  {
    switch (config.option[i].type)
    {
      case ConfigOptionType_None:
        printf("KEY:  %-35s | VALUE: %-10s | TYPE: %-5s\n", CFromStr8(config.option[i].key),
               CFromStr8(config.option[i].value), dtype_names[config.option[i].type]);
        break;
      case ConfigOptionType_I32:
        printf("KEY:  %-35s | VALUE: %-10d | TYPE: %-5s\n", CFromStr8(config.option[i].key),
               config.option[i].v_i32, dtype_names[config.option[i].type]);
        break;
      case ConfigOptionType_F32:
        printf("KEY:  %-35s | VALUE: %-10f | TYPE: %-5s\n", CFromStr8(config.option[i].key),
               config.option[i].v_f32, dtype_names[config.option[i].type]);
        break;
      case ConfigOptionType_B32:
        printf("KEY:  %-35s | VALUE: %-10d | TYPE: %-5s\n", CFromStr8(config.option[i].key),
               config.option[i].v_b32, dtype_names[config.option[i].type]);
        break;
      case ConfigOptionType_Str8:
        printf("KEY:  %-35s | VALUE: %-10s | TYPE: %-5s\n", CFromStr8(config.option[i].key),
               CFromStr8(config.option[i].v_str8), dtype_names[config.option[i].type]);
        break;
      default:
        LOG_ERROR("Unexpected value in config option type");
        break;
    }
  }
}

// config app_config = create_config(["a", "b", "c", "sa"], [int, float, b, b])
// fill_config(app_config, file_path)
// I32 app.window.width = GetConfigOption(&app_config, Str8C("resolution_width"));
