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

const char* dtype_names[] = {"NONE", "INT", "FLOAT", "BOOL", "STR"};

typedef enum
{
  ST_INIT,
  ST_SIGN,
  ST_POINT,
  ST_INT,
  ST_FLOAT,
  ST_STR_OR_BOOL,
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

ConfigOptionTypeEnum check_type(Str8 value);

U16 find_option_pos(Str8 key_to_find, Config config);
I32 get_config_value_int(Str8 key_to_get, Config config);
F32 get_config_value_float(Str8 key_to_get, Config config);
B32 get_config_value_bool(Str8 key_to_get, Config config);
Str8 get_config_value_str8(Str8 key_to_get, Config config);

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

  ConfigOption config_arr[max_config_arr_len];

  Config config = {
      .length = max_config_arr_len,
      .option = config_arr,
  };

  U16 line_n = 0;
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
          LOG_ERROR("SRAKA 1")
          exit(1);
        }
        Str8 value = Str8C(temp_str);
        config.option[line_n].type = check_type(value);
        switch (config.option[line_n].type)
        {
          case ConfigOptionType_None:
            LOG_ERROR("SRAKA 3");
            break;
          case ConfigOptionType_I32:
            config.option[line_n].v_i32 = strtol(CFromStr8(value), NULL, 10);
            break;
          case ConfigOptionType_F32:
            config.option[line_n].v_f32 = strtof(CFromStr8(value), NULL);
            break;
          case ConfigOptionType_B32:
            config.option[line_n].v_b32 = Str8Equal(value, Str8C("true"));
            break;
          case ConfigOptionType_Str8:
            config.option[line_n].v_str8 = CopyStr8(arena, value);
            break;
        }
        config.option[line_n].value = CopyStr8(arena, Str8C(temp_str));
        break;
      }
      if (line.data[i] == ' ')
        continue;
      if (line.data[i] == '=')
      {
        temp_str[line_i] = '\0';
        if (key_value_switch != 0)
        {
          LOG_ERROR("SRAKA 0")
          exit(1);
        }
        config.option[line_n].key = CopyStr8(arena, Str8C(temp_str));
        key_value_switch++;
        line_i = 0;
        continue;
      }

      temp_str[line_i] = line.data[i];
      line_i++;
    }
    line_n++;
  }
  // CHANGE
  config.length = line_n;

  LOG_INFO("\n");
  for (U16 i = 0; i < line_n; i++)
  {
    switch (config.option[i].type)
    {
      case ConfigOptionType_None:
        LOG_ERROR("SRAKA 3");
        break;
      case ConfigOptionType_I32:
        LOG_INFO("KEY:  %-35s | VALUE: %-10d | TYPE: %-5s\n", CFromStr8(config.option[i].key),
                 config.option[i].v_i32, dtype_names[config.option[i].type]);
        break;
      case ConfigOptionType_F32:
        LOG_INFO("KEY:  %-35s | VALUE: %-10f | TYPE: %-5s\n", CFromStr8(config.option[i].key),
                 config.option[i].v_f32, dtype_names[config.option[i].type]);
        break;
      case ConfigOptionType_B32:
        LOG_INFO("KEY:  %-35s | VALUE: %-10d | TYPE: %-5s\n", CFromStr8(config.option[i].key),
                 config.option[i].v_b32, dtype_names[config.option[i].type]);
        break;
      case ConfigOptionType_Str8:
        LOG_INFO("KEY:  %-35s | VALUE: %-10s | TYPE: %-5s\n", CFromStr8(config.option[i].key),
                 CFromStr8(config.option[i].value), dtype_names[config.option[i].type]);
        break;
      default:
        LOG_INFO("SRAKA FINAL");
        break;
    }
  }

  LOG_INFO("\n\n");
  Str8 key = Str8C("app.settings.variable_int");
  LOG_INFO("KEY:  %-35s | VALUE: %-10d\n", CFromStr8(key), get_config_value_int(key, config));
  key = Str8C("app.settings.window.width");
  LOG_INFO("KEY:  %-35s | VALUE: %-10d\n", CFromStr8(key), get_config_value_int(key, config));

  key = Str8C("app.program1.ratio");
  LOG_INFO("KEY:  %-35s | VALUE: %-10f\n", CFromStr8(key), get_config_value_float(key, config));
  key = Str8C("random3");
  LOG_INFO("KEY:  %-35s | VALUE: %-10f\n", CFromStr8(key), get_config_value_float(key, config));

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

  fclose(file);

  return 0;
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

ConfigOptionTypeEnum check_type(Str8 value)
{
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

// config app_config = create_config(["a", "b", "c", "sa"], [int, float, b, b])
// fill_config(app_config, file_path)
// I32 app.window.width = GetConfigOption(&app_config, Str8C("resolution_width"));