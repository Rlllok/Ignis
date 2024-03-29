#pragma once

#include "third_party/vulkan/include/vulkan.h"
#include "third_party/vulkan/include/vulkan_win32.h"

#include "base/base_include.h"

#define NUM_FRAMES_IN_FLIGHT 3

// --AlNov: @NOTE Temporary there
struct R_Mesh
{
    struct MVP {
        alignas(16) Vec3f color;
        alignas(16) Vec3f centerPosition;
    } mvp;

    R_Mesh* next;
    R_Mesh* previous;

    struct Vertex
    {
        Vec3f position;
    };

    Vertex vertecies[4];
    u32 indecies[6];

    VkDescriptorSet mvpSet;
    VkBuffer mvpBuffer;
    VkDeviceMemory mvpBufferMemory;
};

struct R_MeshList
{
    R_Mesh* firstMesh;
    R_Mesh* lastMesh;
    u32 count;
};
global R_MeshList meshList = {};

void R_PushMesh(R_MeshList* list, R_Mesh* mesh);
void R_AddMeshToDrawList(R_Mesh* mesh);
// --AlNov: End Temp Data

global VkInstance R_VK_Instance = VK_NULL_HANDLE;
global VkDebugUtilsMessengerEXT R_VK_DebugMessenger = VK_NULL_HANDLE;

struct R_VK_Device
{
    VkDevice handle = VK_NULL_HANDLE;
    VkPhysicalDevice GPU = VK_NULL_HANDLE;
    u32 queueIndex = 0;
};
global R_VK_Device R_Device;

struct R_VK_WindowResources
{
    VkSwapchainKHR      swapchain;
    VkSurfaceKHR        surface;
    VkSurfaceFormatKHR  surfaceFormat;
    Vec2u               size;
    u32                 imageCount;
    VkImage*            images;
    VkImageView*        imageViews;
    VkFramebuffer*      framebuffers;

    bool bIsWindowResized;
};
global R_VK_WindowResources R_WindowResources;

struct R_VK_Pipeline
{
    VkPipeline handle = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};
global R_VK_Pipeline R_Pipeline;

struct R_VK_CommandPool
{
    VkCommandPool handle = VK_NULL_HANDLE;
    VkCommandBuffer cmdBuffers[NUM_FRAMES_IN_FLIGHT] = {};
};
global R_VK_CommandPool R_CmdPool;

struct R_VK_DescriptorPool
{
    VkDescriptorPool handle = VK_NULL_HANDLE;
};
global R_VK_DescriptorPool R_DescriptorPool;

struct R_VK_MVP
{
    alignas(16) Vec3f centerPosition;
    alignas(16) Vec3f color;
};
global R_VK_MVP R_MVP = {};
global VkBuffer R_MVPBuffer = VK_NULL_HANDLE;
global VkDeviceMemory R_MVPMemory = VK_NULL_HANDLE;

struct R_VK_MVPDescriptor
{
    VkDescriptorSet handle = VK_NULL_HANDLE;
    VkDescriptorSetLayout layout = VK_NULL_HANDLE;
};  
global R_VK_MVPDescriptor R_MVPDescriptor;

struct R_VK_SyncTools
{
    VkFence fences[NUM_FRAMES_IN_FLIGHT] = {};
    VkSemaphore imageIsAvailableSemaphores[NUM_FRAMES_IN_FLIGHT] = {};
    VkSemaphore imageIsReadySemaphores[NUM_FRAMES_IN_FLIGHT] = {};
};
global R_VK_SyncTools R_SyncTools;

struct R_VK_VertexBuffer
{
    VkBuffer buffer;
    VkDeviceMemory memory;
    void* mappedMemory;
    u32 currentPosition;
    u32 size;
};
global R_VK_VertexBuffer R_VertexBuffer;

struct R_VK_IndexBuffer
{
    VkBuffer buffer;
    VkDeviceMemory memory;
    void* mappedMemory;
    u32 currentPosition;
    u32 size;
};
global R_VK_IndexBuffer R_IndexBuffer;

// --AlNov: Functions --------------------------------------------------
void R_VK_CreateInstance();
void R_VK_CreateDevice();
void R_VK_CreateSurface(OS_Window window);
void R_VK_CreateSwapchain();
void R_VK_CreateDescriptorPool();
void R_VK_AllocateDesciptorSet();
void R_VK_CreatePipeline();
void R_VK_CreateFramebuffers();
void R_VK_CreateCommandPool();
void R_VK_AllocateCommandBuffers();
void R_VK_CreateSyncTools();
void R_VK_CreateVertexBuffer();
void R_VK_CreateIndexBuffer();

void R_VK_CleanSwapchainResources();

void R_ResizeWindow();
void R_VK_HandleWindowResize();

// --AlNov: @TODO Handle VK Object destruction

void R_VK_CreateBuffer(VkBufferUsageFlags usage, VkMemoryPropertyFlags propertyFlags, u32 size, VkBuffer* outBuffer, VkDeviceMemory* outMemory);
void R_VK_CopyToMemory(VkDeviceMemory memory, void* data, u32 size);
void R_RecordCmdBuffer(VkCommandBuffer cmdBuffer, u32 imageIndex);
void R_DrawMesh();
void R_EndFrame();

// -- AlNov: Helpers --------------------------------------------------
void R_UpdateWindowData(OS_Window window);
VkExtent2D R_VK_VkExtent2DFromVec2u(Vec2u vec);