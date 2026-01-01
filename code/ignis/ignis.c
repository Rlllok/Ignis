#include "ignis.h"

func Ignis_Scene
Ignis_CreateScene(Arena* arena, U32 max_entity_count)
{
  Ignis_Scene result = {0};

  result.entities = Ignis_EntityArrayAllocate(arena, max_entity_count);
  Ignis_EntityArrayAdd(&result.entities, (Ignis_Entity){0});

  result.entity_hash_map = HashMapI32Allocate(arena, max_entity_count);

  return result;
}

func Ignis_EntityID
Ignis_CreateEntity(Ignis_Scene* scene, Ignis_Entity entity)
{
  Ignis_EntityID result = {0};

  entity.id.id = scene->entities.length;
  Ignis_EntityArrayAdd(&scene->entities, entity);
  HashMapI32Set(&scene->entity_hash_map, entity.name, entity.id.id);

  if (entity.type == Ignis_EntityType_Camera)
  {
    scene->active_camera_id = entity.id;
  }

  return result;
}

func Ignis_Entity*
Ignis_GetEntity(Ignis_Scene* scene, Str8 name)
{
  Ignis_Entity* result = 0;
  
  // 1 January 2026: @TODO
  // Use array id not EntityID
  I32 entity_slot = HashMapI32Get(&scene->entity_hash_map, name);
  if (entity_slot > 0)
  {
    result = Ignis_EntityArrayGetPointer(&scene->entities, entity_slot);
  }

  return result;
}
