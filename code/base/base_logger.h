#pragma once

#include "base_core.h"

enum LogMessageType
{
  LOG_MESSAGE_TYPE_NONE,
  LOG_MESSAGE_TYPE_ERROR,
  LOG_MESSAGE_TYPE_WARNING,
  LOG_MESSAGE_TYPE_INFO,

  LOG_MESSAGE_TYPE_COUNT
};

func void LogOutput(LogMessageType message_type, const char* message, ...);

#ifndef LOG_ERROR
#define LOG_ERROR(message, ...) LogOutput(LOG_MESSAGE_TYPE_ERROR, message, ##__VA_ARGS__);
#endif // LOG_ERROR

#ifndef LOG_WARNING
#define LOG_WARNING(message, ...) LogOutput(LOG_MESSAGE_TYPE_WARNING, message, ##__VA_ARGS__);
#endif // LOG_WARNING

#ifndef LOG_INFO
#define LOG_INFO(message, ...) LogOutput(LOG_MESSAGE_TYPE_INFO, message, ##__VA_ARGS__);
#endif // LOG_INFO

func void AssertionFail(const char* expression, const char* message, const char* file_name, u32 line_number);

#define ASSERTION_ENABLED

#ifdef ASSERTION_ENABLED
  #if _MSC_VER
    #include <intrin.h>
    #define debugBreak() __debugbreak()
  #else
    #define debugBreak() __builtin_trap()
  #endif // _MSC_VER

  #define ASSERT(expression)                                \
    {                                                       \
      if (expression)                                       \
      {                                                     \
        AssertionFail(#expression, "", __FILE__, __LINE__); \
        debugBreak();                                       \
      }                                                     \
    }

  #define ASSERT_MESSAGE(expression, message)                    \
    {                                                            \
      if (expression)                                            \
      {                                                          \
        AssertionFail(#expression, message, __FILE__, __LINE__); \
        debugBreak();                                            \
      }                                                          \
    }

#else
  #define ASSERT(expression)
  #define ASSERT_MESSAGE(expression)
#endif // ASSERTION_ENABLED
