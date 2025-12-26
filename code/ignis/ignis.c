#include "ignis.h"

func Ignis_Scene
Ignis_CreateScene(Arena* arena, U32 max_entity_count)
{
  Ignis_Scene result = {0};

  result.entities = Ignis_EntityArrayAllocate(arena, max_entity_count);
  Ignis_EntityArrayAdd(&result.entities, (Ignis_Entity){0});

  return result;
}

func Ignis_ID
Ignis_CreateEntity(Ignis_Scene* scene, Ignis_Entity entity)
{
  Ignis_ID result = 0;

  entity.id = scene->entities.length;
  Ignis_EntityArrayAdd(&scene->entities, entity);

  return result;
}
