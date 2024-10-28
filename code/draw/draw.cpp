#include "draw.h"

func void
D_Init(Arena* arena)
{
  // --AlNov: Create quad Vertex and Index Buffers
  {
    Vec2f vertecies[] = {
      MakeVec2f(-1.0f, -1.0f),
      MakeVec2f(+1.0f, -1.0f),
      MakeVec2f(+1.0f, +1.0f),
      MakeVec2f(-1.0f, +1.0f),
    };
    _d_state.quad_vertex_buffer = R_CreateVertexBuffer(
      vertecies,
      sizeof(vertecies[0]),
      CountArrayElements(vertecies)
    );
    U32 indecies[] = {0, 1, 2, 2, 3, 0};
    _d_state.quad_index_buffer = R_CreateIndexBuffer(
      indecies,
      sizeof(indecies[0]),
      CountArrayElements(indecies)
    );
  }

  // --AlNov: Create SDF 2D Box pipeline
  {
    R_VertexAttributeFormat vertex_attributes[] = {
      R_VERTEX_ATTRIBUTE_FORMAT_VEC2F
    };
    R_PipelineAssignAttributes(
        &_d_state.box_pipeline,
        vertex_attributes,
        CountArrayElements(vertex_attributes));

    R_BindingInfo scene_bindings[] = 
    {
      {R_BINDING_TYPE_UNIFORM_BUFFER, R_SHADER_TYPE_VERTEX}
    };
    R_PipelineAssignSceneBindingLayout(
        &_d_state.box_pipeline, scene_bindings, CountArrayElements(scene_bindings));

    R_BindingInfo instance_bindings[] = 
    {
      {R_BINDING_TYPE_UNIFORM_BUFFER, R_SHADER_TYPE_VERTEX}
    };
    R_PipelineAssignInstanceBindingLayout(
        &_d_state.box_pipeline, instance_bindings, CountArrayElements(instance_bindings));

    R_H_LoadShader(
        arena, "data/shaders/sdf/sdf_vs.glsl",
        "main", R_SHADER_TYPE_VERTEX,
        &_d_state.box_pipeline.shaders[R_SHADER_TYPE_VERTEX]);
    R_H_LoadShader(
        arena, "data/shaders/sdf/sdf_box_fs.glsl",
        "main", R_SHADER_TYPE_FRAGMENT,
        &_d_state.box_pipeline.shaders[R_SHADER_TYPE_FRAGMENT]);

    _d_state.box_pipeline.is_depth_test_enabled = false;

    Renderer.CreatePipeline(&_d_state.box_pipeline);
  }

  // --AlNov: Create SDF 2D Circle pipeline
  {
    R_VertexAttributeFormat vertex_attributes[] = {
      R_VERTEX_ATTRIBUTE_FORMAT_VEC2F
    };
    R_PipelineAssignAttributes(
        &_d_state.circle_pipeline, vertex_attributes,
        CountArrayElements(vertex_attributes));

    R_BindingInfo scene_bindings[] = 
    {
      {R_BINDING_TYPE_UNIFORM_BUFFER, R_SHADER_TYPE_VERTEX}
    };
    R_PipelineAssignSceneBindingLayout(&_d_state.circle_pipeline, scene_bindings, CountArrayElements(scene_bindings));

    R_BindingInfo instance_bindings[] = 
    {
      {R_BINDING_TYPE_UNIFORM_BUFFER, R_SHADER_TYPE_VERTEX}
    };
    R_PipelineAssignInstanceBindingLayout(&_d_state.circle_pipeline, instance_bindings, CountArrayElements(instance_bindings));

    R_H_LoadShader(
        arena, "data/shaders/sdf/sdf_vs.glsl",
        "main", R_SHADER_TYPE_VERTEX,
        &_d_state.circle_pipeline.shaders[R_SHADER_TYPE_VERTEX]);
    R_H_LoadShader(
        arena, "data/shaders/sdf/sdf_circle_fs.glsl",
        "main", R_SHADER_TYPE_FRAGMENT,
        &_d_state.circle_pipeline.shaders[R_SHADER_TYPE_FRAGMENT]);

    _d_state.circle_pipeline.is_depth_test_enabled = false;

    Renderer.CreatePipeline(&_d_state.circle_pipeline);
  }
}

func void
D_DrawRectangle(RectI rectangle, Vec3f color)
{
  struct
  {
    alignas(16) Mat4x4f projection;
  } scene_data;
  scene_data.projection = MakeOrthographic4x4f(
      0.0f, 1280.0f, 0.0f, 720.0f, 0.0f, 1.0f
      );

  struct
  {
    alignas(8)  Vec2f translate;
    alignas(8)  Vec2f size;
    alignas(16) Vec3f color;
  } draw_vs_data;
  draw_vs_data.translate.x = rectangle.position.x;
  draw_vs_data.translate.y = rectangle.position.y;
  draw_vs_data.size.x = rectangle.size.x;
  draw_vs_data.size.y = rectangle.size.y;
  draw_vs_data.color = color;

  struct
  {
    alignas(16) Vec3f color;
  } draw_fs_data;
  draw_fs_data.color = color;

  R_DrawInfo draw_info = {};
  draw_info.pipeline = &_d_state.box_pipeline;
  draw_info.vertex_buffer = &_d_state.quad_vertex_buffer;
  draw_info.index_buffer = &_d_state.quad_index_buffer;
  draw_info.scene_group.data = &scene_data;
  draw_info.scene_group.data_size = sizeof(scene_data);
  draw_info.instance_group.data = &draw_vs_data;
  draw_info.instance_group.data_size = sizeof(draw_vs_data);

  RectI viewport = {};
  viewport.x = 0;
  viewport.y = 0;
  viewport.w = 1280;
  viewport.h = 720;
  draw_info.viewport = viewport;

  RectI scissor = rectangle;
  draw_info.scissor = scissor;

  Renderer.Draw(&draw_info);

}

func void
D_DrawCircle(Vec2I position, I32 radius, Vec3f color)
{
  struct
  {
    alignas(16) Mat4x4f projection;
  } scene_data;
  scene_data.projection = MakeOrthographic4x4f(
    0.0f, 1280.0f, 0.0f, 720.0f, 0.0f, 1.0f
  );

  struct
  {
    alignas(8)  Vec2f translate;
    alignas(8)  Vec2f size;
    alignas(16) Vec3f color;
  } draw_vs_data;
  draw_vs_data.translate = Vec2FFromVec(position);
  draw_vs_data.size = MakeVec2f(radius, radius);
  draw_vs_data.color = color;

  struct
  {
    alignas(16) Vec3f color;
  } draw_fs_data;
  draw_fs_data.color = color;

  R_DrawInfo draw_info = {};
  draw_info.pipeline = &_d_state.circle_pipeline;
  draw_info.vertex_buffer = &_d_state.quad_vertex_buffer;
  draw_info.index_buffer = &_d_state.quad_index_buffer;
  draw_info.scene_group.data = &scene_data;
  draw_info.scene_group.data_size = sizeof(scene_data);
  draw_info.instance_group.data = &draw_vs_data;
  draw_info.instance_group.data_size = sizeof(draw_vs_data);

  RectI viewport = {};
  viewport.x = 0;
  viewport.y = 0;
  viewport.w = 1280;
  viewport.h = 720;
  draw_info.viewport = viewport;

  RectI scissor = {};
  scissor.position.x = Max(0, position.x - radius);
  scissor.position.y = Max(0, position.y - radius);
  scissor.size = MakeVec2I(radius*2.0f, radius*2.0f);
  draw_info.scissor = scissor;

  Renderer.Draw(&draw_info);
}
