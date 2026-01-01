#pragma once

typedef struct Ignis_R_State Ignis_R_State;
struct Ignis_R_State
{
  Arena* arena;

  OS_Window* window;

  R_Buffer data_buffer;
  R_Buffer transfer_buffer;
  
  R_CommandBuffer command_buffer;

  R_Texture swapchain;

  R_GraphicsPipeline grid_pipeline;
  R_GraphicsPipeline depth_prepass_pipeline;
  R_GraphicsPipeline mesh_pipeline;
  R_GraphicsPipeline joint_pipeline;

  R_TextureSampler texture_sampler;
  R_Texture default_color_texture;
  R_Texture mesh_color_texture;
  R_Texture mesh_normal_texture;

  R_Texture depth_texture; // -AlNov: @TODO should it be created for R_VK_Swapchain?
  R_Texture test_texture;
} _ignis_r_state;

func void Ignis_R_Init(R_RendererType type, OS_Window* window);

func R_Texture Ignis_R_CreateLoadTexture(R_Buffer buffer, Str8 path, R_TextureFormat format);

func void Ignis_R_PreparePipelines();
func void Ignis_R_PrepareTextures();

// -------------------------------------------------------------------
// -- Render ---------------------------------------------------------
func void Ignis_R_BeginFrame();
func void Ignis_R_EndFrame();

func void Ignis_R_RenderScene(Ignis_Scene* scene);
func void Ignis_R_RenderUI(UI_DrawCommandArray commands);

func void Ignis_R_RenderGrid         (Ignis_Scene* scene, R_ColorTarget color, R_DepthStencilTarget depth);
func void Ignis_R_RenderEntityPrepass(Ignis_Entity* camera, Ignis_Entity* entity);
func void Ignis_R_RenderEntity       (Ignis_Entity* camera, Ignis_Entity* entity);
