#pragma once

#include "base/base_include.h"

// -------------------------------------------------------------------
// -- Entity ---------------------------------------------------------
typedef struct Ignis_EntityID Ignis_EntityID;
struct Ignis_EntityID
{
  I32 id;
  I32 generation;
};

func B32 Ignis_EntityIDValid(Ignis_EntityID id)                  {return id.id > 0;}
func B32 Ignis_EntityIDEqual(Ignis_EntityID a, Ignis_EntityID b) {return a.id == b.id && a.generation == b.generation;}

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
  Ignis_EntityID   id;
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
    } camera;
  };
};
Ignis_Entity _ignis_entity_nil = {0};
DefineArray(Ignis_Entity, Ignis_EntityArray, _ignis_entity_nil)

func B32 Ignis_EntityValid(Ignis_Entity* entity) {return Ignis_EntityIDValid(entity->id);}

typedef struct Ignis_Scene Ignis_Scene;
struct Ignis_Scene
{
  Ignis_EntityArray entities;
  HashMapI32  entity_hash_map;

  Ignis_EntityID active_camera_id;
  Ignis_EntityID selected_entity_id;
};

func Ignis_Scene    Ignis_CreateScene (Arena* arena, U32 max_entity_count);
func Ignis_EntityID Ignis_CreateEntity(Ignis_Scene* scene, Ignis_Entity entity);
func Ignis_Entity*  Ignis_GetEntity   (Ignis_Scene* scene, Str8 name);

func Ignis_Entity* Ignis_GetCamera        (Ignis_Scene* scene) {return Ignis_EntityArrayGetPointer(&scene->entities, scene->active_camera_id.id);}
func Ignis_Entity* Ignis_GetSelectedEntity(Ignis_Scene* scene) {return Ignis_EntityArrayGetPointer(&scene->entities, scene->selected_entity_id.id);}
