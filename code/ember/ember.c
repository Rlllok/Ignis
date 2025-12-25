#include "base/base_include.h"
#include "os/os_include.h"

#include "base/base_include.c"
#include "os/os_include.c"

#include <stdio.h>
#include <stdlib.h>

typedef struct Ember_State Ember_State;
struct Ember_State
{
  Arena* arena;
} ember_state;

func void
Ember_Init()
{
  ember_state.arena = AllocateArena(Megabytes(16));
}

func Str8
Ember_ReadEntireFile(Str8 file_name)
{
  Str8 result = {0};

  FILE* file = fopen(CFromStr8(file_name), "r");
  if (file)
  {
    fseek(file, 0, SEEK_END);
    U64 file_size = ftell(file);
    LOG_DEBUG("File size: %d\n", file_size);
    fseek(file, 0, SEEK_SET);

    result = AllocateStr8(ember_state.arena, file_size);
    fread(result.data, file_size, 1, file);

    fclose(file);
  }
  
  return result;
}

typedef U16 Ember_TokenType;
enum Ember_TokenTypeEnum
{
  Ember_TokenType_None,
  // One Symbol
  Ember_TokenType_OpenParanthesis,
  Ember_TokenType_CloseParanthesis,
  Ember_TokenType_Comma,
  Ember_TokenType_End,
  
  Ember_TokenType_Identifier,
  Ember_TokenType_Number,
} Ember_TokenTypeEnum;

typedef struct Ember_Token Ember_Token;
struct Ember_Token
{
  Str8 str;
  Ember_TokenType type;
};

typedef struct Ember_Tokenizer Ember_Tokenizer;
struct Ember_Tokenizer
{
  Str8 content;
  U64  position;
};

func void
Ember_SkipSymbols(Ember_Tokenizer* tokenizer)
{
  while (1)
  {
    if (IsWhitespace(tokenizer->content, tokenizer->position))
    {
      tokenizer->position += 1;
    }
    else if(Str8GetSymbol(tokenizer->content, tokenizer->position) == '/' && Str8GetSymbol(tokenizer->content, tokenizer->position + 1) == '/')
    {
      while (Str8GetSymbol(tokenizer->content, tokenizer->position) != 0 && !IsLineEnd(tokenizer->content, tokenizer->position))
      {
        tokenizer->position += 1;
      }
    }
    else if (Str8GetSymbol(tokenizer->content, tokenizer->position) == '/' && Str8GetSymbol(tokenizer->content, tokenizer->position + 1) == '*')
    {
      LOG_DEBUG("Multiline Comment\n");
      while (
        !(Str8GetSymbol(tokenizer->content, tokenizer->position) == '*'
        && Str8GetSymbol(tokenizer->content, tokenizer->position + 1) == '/')
      )
      {
        tokenizer->position += 1;
      }
      tokenizer->position += 2; // skip '*' and '/'
    }
    else
    {
      break;
    }
  }
}

func Ember_Token
Ember_GetToken(Ember_Tokenizer* tokenizer)
{
  Ember_SkipSymbols(tokenizer);

  Ember_Token result = {
    .str = {
      .data = tokenizer->content.data + tokenizer->position,
      .length = 1,
    }
  };

  U8 symbol = tokenizer->content.data[tokenizer->position];
  switch (symbol)
  {
    case '\0': {result.type = Ember_TokenType_End;              tokenizer->position += 1;} break;
    case  '(': {result.type = Ember_TokenType_OpenParanthesis;  tokenizer->position += 1;} break;
    case  ')': {result.type = Ember_TokenType_CloseParanthesis; tokenizer->position += 1;} break;
    case  ',': {result.type = Ember_TokenType_Comma;            tokenizer->position += 1;} break;

    default:
    {
      if (IsAlphabet(tokenizer->content, tokenizer->position))
      {
        result.type = Ember_TokenType_Identifier;

        U64 start_position = tokenizer->position;

        while (IsAlphabet(tokenizer->content, tokenizer->position)
          || IsDigit(tokenizer->content, tokenizer->position)
          || Str8GetSymbol(tokenizer->content, tokenizer->position) == '_'
        )
        {
          tokenizer->position += 1;
        }

        result.str = SubStr8(ember_state.arena, tokenizer->content, start_position, tokenizer->position - start_position);
      }
      else if (IsDigit(tokenizer->content, tokenizer->position))
      {
        result.type = Ember_TokenType_Number;
        
        U64 start_position = tokenizer->position;

        while (IsDigit(tokenizer->content, tokenizer->position))
        {
          tokenizer->position += 1;
        }

        result.str = SubStr8(ember_state.arena, tokenizer->content, start_position, tokenizer->position - start_position);
      }
      else
      {
        result.type = Ember_TokenType_None;

        result.str = (Str8){0};
        tokenizer->position += 1;
      }
    } break;
  }

  return result;
}

func void
Ember_ParseTweakB32(Ember_Tokenizer* tokenizer)
{ 
  if (Ember_GetToken(tokenizer).type == Ember_TokenType_OpenParanthesis)
  {
    Ember_Token name = Ember_GetToken(tokenizer);
    if (name.type == Ember_TokenType_Identifier)
    {
      if (Ember_GetToken(tokenizer).type == Ember_TokenType_Comma)
      {
        Ember_Token value = Ember_GetToken(tokenizer);
        if (value.type == Ember_TokenType_Number)
        {
          if (Ember_GetToken(tokenizer).type == Ember_TokenType_CloseParanthesis)
          {
            LOG_DEBUG("B32 ember_tweak_b32_%.*s = %.*s;\n", name.str.length, name.str.data, value.str.length, value.str.data);
          }
        }
      }
    }
  }
}


I32 main()
{
  LOG_DEBUG("=== Ember Start ===\n");
  Ember_Init();

  Ember_Tokenizer tokenizer = {0};
  tokenizer.content = Ember_ReadEntireFile(Str8C("../code/ember/ember_test.c"));

  B32 parsing = 1;
  while (parsing)
  {
    Ember_Token token = Ember_GetToken(&tokenizer);

    switch (token.type)
    {
      default:
      {
        // LOG_DEBUG("%d: %.*s\n", token.type, token.str.length, token.str.data);
      } break;

      case Ember_TokenType_None:
      {
      } break;

      case Ember_TokenType_End:
      {
        parsing = 0;
      } break;

      case Ember_TokenType_Identifier:
      {
        if (Str8Equal(Str8C("Ember_TweakB32"), token.str))
        {
          Ember_ParseTweakB32(&tokenizer);
        }
      } break;
    }
  }

  LOG_DEBUG("=== Ember end ===\n");
  return 0;
}
