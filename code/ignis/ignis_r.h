#pragma once

typedef struct Ignis_R_State Ignis_R_State;
struct Ignis_R_State
{
  Arena* arena;

  OS_Window* window;

  RHI_Buffer data_buffer;
  RHI_Buffer transfer_buffer;
  
  RHI_CommandBuffer command_buffer;

  RHI_Texture swapchain;

  RHI_GraphicsPipeline grid_pipeline;
  RHI_GraphicsPipeline depth_prepass_pipeline;
  RHI_GraphicsPipeline shadow_map_pipeline;
  RHI_GraphicsPipeline mesh_pipeline;
  RHI_GraphicsPipeline joint_pipeline;

  RHI_TextureSampler texture_sampler;
  RHI_Texture default_color_texture;
  RHI_Texture mesh_color_texture;
  RHI_Texture mesh_normal_texture;

  RHI_Texture depth_texture; // -AlNov: @TODO should it be created for RHI_VK_Swapchain?
  RHI_Texture test_texture;

  RHI_Texture shadow_map;
} _ignis_r_state;

func void Ignis_R_Init(RHI_RendererType type, OS_Window* window);

func RHI_Texture Ignis_R_CreateLoadTexture(RHI_Buffer buffer, Str8 path, RHI_TextureFormat format);

func void Ignis_R_PreparePipelines();
func void Ignis_R_PrepareTextures();

// -------------------------------------------------------------------
// -- Render ---------------------------------------------------------
func void Ignis_R_BeginFrame();
func void Ignis_R_EndFrame();

func void Ignis_R_RenderScene(Ignis_Scene* scene);
func void Ignis_R_RenderUI(UI_DrawCommandArray commands);

func void Ignis_R_RenderGrid         (Ignis_Scene* scene, RHI_ColorTarget color, RHI_DepthStencilTarget depth);
func void Ignis_R_ShadowMapPass      (Ignis_Scene* scene, Ignis_Entity* entity);
func void Ignis_R_RenderEntityPrepass(Ignis_Entity* camera, Ignis_Entity* entity);
func void Ignis_R_RenderEntity       (Ignis_Entity* camera, Ignis_Entity* entity, B32 selected);
