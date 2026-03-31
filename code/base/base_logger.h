#pragma once

#include "base_core.h"

typedef U16 LogMessageKind;
enum LogMessageKindEnum {
  LogMessageKind_None,
  LogMessageKind_Error,
  LogMessageKind_Warning,
  LogMessageKind_Info,
  LogMessageKind_Debug,
  
  LogMessageKind_Count
} LogMessageKindEnum;

func void LogOutput(LogMessageKind message_kind, const char* message, ...);

#ifndef  LogText
#define LogText(message, ...) LogOutput(LogMessageKind_None, message, ##__VA_ARGS__);
#endif // LogText

#ifndef LogError
#define LogError(message, ...) LogOutput(LogMessageKind_Error, message, ##__VA_ARGS__);
#endif // LogError

#ifndef LogWarning
#define LogWarning(message, ...) LogOutput(LogMessageKind_Warning, message, ##__VA_ARGS__);
#endif // LogWarning

#ifndef LogInfo
#define LogInfo(message, ...) LogOutput(LogMessageKind_Info, message, ##__VA_ARGS__);
#endif // LogInfo

#if IGNIS_DEBUG
#define LogDebug(message, ...) LogOutput(LogMessageKind_Debug, message, ##__VA_ARGS__);
#else
#define LogDebug(message, ...)
#endif // IGNIS_DEBUG

func void AssertionFail(const char* expression, const char* message, const char* file_name, U32 line_number);

#define ASSERTION_ENABLED

#ifdef ASSERTION_ENABLED
  #if _MSC_VER
    #include <intrin.h>
    #define debugBreak() __debugbreak()
  #else
    #define debugBreak() __builtin_trap()
  #endif // _MSC_VER

  #define Assert(expression)                                \
    {                                                       \
      if (!(expression))                                       \
      {                                                     \
        AssertionFail(#expression, "", __FILE__, __LINE__); \
        debugBreak();                                       \
      }                                                     \
    }

  #define AssertMessage(expression, message)                    \
    {                                                            \
      if (!(expression))                                         \
      {                                                          \
        AssertionFail(#expression, message, __FILE__, __LINE__); \
        debugBreak();                                            \
      }                                                          \
    }

#else
  #define ASSERT(expression)
  #define ASSERT_MESSAGE(expression)
#endif // ASSERTION_ENABLED
