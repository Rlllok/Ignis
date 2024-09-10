#include "base/base_include.h"
#include "base/base_include.cpp"

// --AlNov: Resources
//      * https://ajmmertens.medium.com/building-an-ecs-1-where-are-my-entities-and-components-63d07c7da742

#define MAX_ENTITIES_NUMBER   64
#define MAX_COMPONENTS_TYPE_NUMBER 16
#define MAX_ARCHETYPES_NUMBER 16

typedef u32 EntityId;

enum ComponentType
{
  COMPONENT_TYPE_NONE,

  COMPONENT_TYPE_Name,
  COMPONENT_TYPE_Position,
  COMPONENT_TYPE_Health,

  COMPONENT_TYPE_COUNT
};

struct NameComponent
{
  const char* name;
};

struct HealthComponent
{
  u32 health = 5;
};

struct PositionComponent
{
  u32 x;
  u32 y;
};

struct ComponentArray
{
  void* elements;
  u32   elemet_size;
  u32   count;
};

struct Archetype
{
  u32 id;
  ComponentType component_types[COMPONENT_TYPE_COUNT];
};

struct Entity
{
  EntityId entity_id;
  Archetype archetype;
  u32 archetype_component_index;
};

struct World
{
  Entity entities[MAX_ENTITIES_NUMBER];
  u32    entity_count;

  Archetype archetypes[MAX_ARCHETYPES_NUMBER];
  u32       archetype_count;
};

func EntityId CreateEntity(World* world);
func Archetype* CreateArchetype(World* world, ComponentType* type_array, u32 count);

func bool HasComponent(World* world, EntityId entity, ComponentType component_type);
func void AddComponent(World* world, EntityId entity, ComponentType component_type);

i32 main()
{
  LOG_INFO("---ECS Test---\n");

  World world = {};

  const EntityId first_entity = CreateEntity(&world);
  
  AddComponent(&world, first_entity, COMPONENT_TYPE_Health);

  ComponentType types[] = { COMPONENT_TYPE_Health };
  Archetype* health_archetype = CreateArchetype(&world, types, 1);

  // world.entities[first_entity].archetype = *health_archetype;
  if (HasComponent(&world, first_entity, COMPONENT_TYPE_Health))
  {
    LOG_INFO("Entity has Health component\n");
  }
  else
  {
    LOG_INFO("Entity has no Health component\n");
  }

  return 0;
}

func EntityId
CreateEntity(World* world)
{
  EntityId entity_id = world->entity_count;
  
  world->entity_count += 1;

  return entity_id;
}

func Archetype*
CreateArchetype(World* world, ComponentType* type_array, u32 count)
{
  Archetype* archetype = &world->archetypes[world->archetype_count];

  archetype->id = world->archetype_count;
  for (i32 i = 0; i < count; i += 1)
  {
    ComponentType* type = type_array + i;
    archetype->component_types[*type] = *type;
  }

  world->archetype_count += 1;

  return archetype;
}

func bool
HasComponent(World* world, EntityId entity, ComponentType component_type)
{
  const Archetype* archetype = &world->entities[entity].archetype;
  
  return archetype->component_types[component_type] != 0;
}

func void
AddComponent(World* world, EntityId entity, ComponentType component_type)
{
  Archetype* archetype = &world->entities[entity].archetype;
  archetype->component_types[component_type] = component_type;
}
