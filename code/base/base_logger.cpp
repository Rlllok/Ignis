#include "base_logger.h"

#include <stdio.h>
#include <memory.h>
#include <stdarg.h>

// --AlNov: @TODO Change char* to Str8

func void
LogOutput(LogMessageType message_type, const char* message, ...)
{
  const char* type_strings[LOG_MESSAGE_TYPE_COUNT] = { "[NONE]: ", "[ERROR]: ", "[WARN]: ", "[INFO]: " };

  const U32 output_message_size = 16000;
  char output_message[output_message_size];
  memset(output_message, 0, sizeof(output_message));

  __builtin_va_list arg_ptr;
  va_start(arg_ptr, message);
  vsnprintf(output_message, output_message_size, message, arg_ptr);
  va_end(arg_ptr);

  char final_message[output_message_size];
  sprintf(final_message, "%s%s", type_strings[message_type], output_message);

  printf("%s", final_message);
}

func void
AssertionFail(const char* expression, const char* message, const char* file_name, U32 line_number)
{
  LogOutput(LOG_MESSAGE_TYPE_ERROR, "Assertion failure: %s, message: %s. File: %s. Line: %d\n", expression, message, file_name, line_number);
}
