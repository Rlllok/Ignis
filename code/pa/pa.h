#pragma once

#include "base/base_include.h"
#include "ignis/ignis.h"

typedef struct PA_Area PA_Area;
struct PA_Area
{
  Ignis_Scene scene;
};

func void PA_StartArea(PA_Area* area);
func void PA_EndArea(PA_Area* area);

typedef struct PA_State PA_State;
struct PA_State
{
  Arena* arena;

  F32 grid_size;

  PA_Area area;
}_pa_state;

func void PA_Init();

func void PA_Input (F32 dt);
func void PA_Update(F32 dt);
