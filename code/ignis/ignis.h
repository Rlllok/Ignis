#pragma once

#include "base/base_include.h"

// -------------------------------------------------------------------
// -- Entity ---------------------------------------------------------
typedef I32 Ignis_ID; // --AlNov 26 December 2025: @TODO I32 or U32 ???
#define Ignis_EntityID_Nil 0

typedef U16 Ignis_EntityType;
enum Ignis_EntityTypeEnum
{
  Ignis_EntityType_None,
  Ignis_EntityType_Actor,
  Ignis_EntityType_Camera,
  Ignis_EntityType_Count,
} Ignis_EntityTypeEnum;

typedef struct Ignis_Entity Ignis_Entity;
struct Ignis_Entity
{
  Ignis_ID         id;
  Ignis_EntityType type; 
  Str8             name;

  Transform transform;

  union
  {
    struct
    {
      F32            smoothness;
      AST_StaticMesh mesh;
      R_Texture      color_texture;
    } actor;

    struct
    {
      Vec3F32 front;
      Vec3F32 right;
      Vec3F32 up;
      F32     yaw;
      F32     pitch;
    };
  };
};
Ignis_Entity _ignis_entity_nil = {0};
DefineArray(Ignis_Entity, Ignis_EntityArray, _ignis_entity_nil)

func B32 Ignis_CheckEntity(Ignis_Entity* entity) {return entity->id <= Ignis_EntityID_Nil;}

typedef struct Ignis_Scene Ignis_Scene;
struct Ignis_Scene
{
  Ignis_EntityArray entities;

  Ignis_ID camera_id;
  Ignis_ID selected_entity_id;
};

func Ignis_Scene Ignis_CreateScene (Arena* arena, U32 max_entity_count);
func Ignis_ID    Ignis_CreateEntity(Ignis_Scene* scene, Ignis_Entity entity);

func Ignis_Entity* Ignis_GetCamera        (Ignis_Scene* scene) {return Ignis_EntityArrayGetPointer(&scene->entities, scene->camera_id);}
func Ignis_Entity* Ignis_GetSelectedEntity(Ignis_Scene* scene) {return Ignis_EntityArrayGetPointer(&scene->entities, scene->selected_entity_id);}
