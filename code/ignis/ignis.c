#include "ignis.h"

func Ignis_Scene
Ignis_CreateScene(Arena* arena, U32 max_entity_count)
{
  Ignis_Scene result = {0};

  result.entities = Ignis_EntityArrayAllocate(arena, max_entity_count);
  Ignis_EntityArrayAdd(&result.entities, (Ignis_Entity){0});

  return result;
}

func Ignis_EntityID
Ignis_CreateEntity(Ignis_Scene* scene, Ignis_Entity entity)
{
  Ignis_EntityID result = {0};

  entity.id.id = scene->entities.length;
  Ignis_EntityArrayAdd(&scene->entities, entity);

  if (entity.type == Ignis_EntityType_Camera)
  {
    scene->active_camera_id = entity.id;
  }

  return result;
}
