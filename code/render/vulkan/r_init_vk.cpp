#include "r_init_vk.h"

#include "stdio.h"

#pragma comment(lib, "third_party/vulkan/lib/vulkan-1.lib")

global Arena* R_Arena = AllocateArena(Kilobytes(20));

// --AlNov: Vulkan Debbuger stuff
VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
    printf("VK_VALIDATION: %s\n\n", pCallbackData->pMessage);

	return VK_FALSE;
}

void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& messengerInfo)
{
	messengerInfo = {};
	messengerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	messengerInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	messengerInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
	messengerInfo.pfnUserCallback = debugCallback;
	messengerInfo.pUserData = nullptr;
}

VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMesseneger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

	if (func != nullptr)
    {
		return func(instance, pCreateInfo, pAllocator, pDebugMesseneger);
	}
	else
    {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

	if (func != nullptr)
    {
		func(instance, debugMessenger, pAllocator);
	}
}

VkResult createDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT* debugMessenger)
{
	VkDebugUtilsMessengerCreateInfoEXT messengerInfo = {};
	populateDebugMessengerCreateInfo(messengerInfo);

	return createDebugUtilsMessengerEXT(instance, &messengerInfo, nullptr, debugMessenger);
}

R_SquareList R_GetSquareList(Arena* arena)
{
    // --AlNov: @NOTE Is it even needed
}

void R_PushSquare(R_SquareList* list, R_Square* square)
{
    if (list->count == 0)
    {
        list->firstSquare = square;
        list->lastSquare = square;
        list->count = 1;

        square->next = 0;
        square->previous = 0;
    }
    else
    {
        square->previous = list->lastSquare;
        square->next = 0;
        list->lastSquare->next = square;
        list->lastSquare = square;
        ++list->count;
    }
}

void R_AddSquareToDrawList(R_Square* square)
{
    R_PushSquare(&squareList, square);
}

// --AlNov: Vulkan Renderer
void R_VK_CreateInstance()
{
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "VulkanRenderingFramework";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "RenderingEngine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    const char* extensionNames[] = {
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
        "VK_KHR_win32_surface",
        "VK_KHR_surface",
    };

    const char* validationLayers[] = {
        "VK_LAYER_KHRONOS_validation",
        "VK_LAYER_LUNARG_monitor",
    };

    VkInstanceCreateInfo instanceInfo = {};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo = &appInfo;
    instanceInfo.enabledLayerCount = CountArrayElements(validationLayers);
    instanceInfo.ppEnabledLayerNames = validationLayers;
    instanceInfo.enabledExtensionCount = CountArrayElements(extensionNames);
    instanceInfo.ppEnabledExtensionNames = extensionNames;

    VkDebugUtilsMessengerCreateInfoEXT messengerInfo;
    populateDebugMessengerCreateInfo(messengerInfo);
    instanceInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&messengerInfo;

    vkCreateInstance(&instanceInfo, nullptr, &R_VK_Instance);

    createDebugMessenger(R_VK_Instance, &R_VK_DebugMessenger);
}

void R_VK_CreateDevice()
{
    Arena* tempArena = AllocateArena(Kilobytes(10));
    {
        u32 deviceCount = 0;
        vkEnumeratePhysicalDevices(R_VK_Instance, &deviceCount, nullptr);
        VkPhysicalDevice* devices = (VkPhysicalDevice*)PushArena(tempArena, deviceCount * sizeof(VkPhysicalDevice));
        vkEnumeratePhysicalDevices(R_VK_Instance, &deviceCount, devices);
        
        for (u32 i = 0; i < deviceCount; ++i)
        {
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(devices[i], &properties);

            if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            {
                R_Device.GPU = devices[i];
            }
        }

        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(R_Device.GPU, &properties);

        printf("GPU NAME: %s\n", properties.deviceName);

        u32 queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(R_Device.GPU, &queueFamilyCount, nullptr);
        VkQueueFamilyProperties* queueFamilyProperties = (VkQueueFamilyProperties*)PushArena(tempArena, queueFamilyCount * sizeof(VkQueueFamilyProperties));
        vkGetPhysicalDeviceQueueFamilyProperties(R_Device.GPU, &queueFamilyCount, queueFamilyProperties);

        for (u32 i = 0; i < queueFamilyCount; ++i)
        {
            if (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                R_Device.queueIndex = i;
                break;
            }
        }

        VkDeviceQueueCreateInfo graphicsQueueInfo = {};
        graphicsQueueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        graphicsQueueInfo.queueFamilyIndex = R_Device.queueIndex;
        graphicsQueueInfo.queueCount = 1;
        const float priority = 1.0f;
        graphicsQueueInfo.pQueuePriorities = &priority;

        const u32 extensionCount = 1;
        const char* extensionNames[extensionCount] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

        VkDeviceCreateInfo deviceInfo = {};
        deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceInfo.queueCreateInfoCount = 1;
        deviceInfo.pQueueCreateInfos = &graphicsQueueInfo;
        deviceInfo.enabledExtensionCount = extensionCount;
        deviceInfo.ppEnabledExtensionNames = extensionNames;
        deviceInfo.pEnabledFeatures = nullptr;

        vkCreateDevice(R_Device.GPU, &deviceInfo, nullptr, &R_Device.handle);
    }
    FreeArena(tempArena);
}

void R_VK_CreatePipeline(const i32 width, const i32 height)
{
    // --AlNov: Vertex Shader
    
    // --AlNov: @TODO Compiler says that fopen is not safe to use
    // It recomends to use the fopen_s function intead.
    // I should read about this more

    // -- AlNov: @NOTE Reading files for Vulkan is total mess.
    // I confused with sizes that modules require...
    // Castiong to u32 as specification sugest is nuts.
    // As I can understand it can be not so ease as (u32*)
    // Should check, but no just go to next stage
    // maybe this can help: https://github.com/lonelydevil/vulkan-tutorial-C-implementation

    Arena* tempArena = AllocateArena(Kilobytes(4000));
    {
        const char* vertexPath = "data/shaders/triangleVS.spv";
        FILE* vertexFile = nullptr;
        vertexFile = fopen(vertexPath, "rb");
        if (!vertexFile)
        {
            printf("Cannot open file %s\n", vertexPath);
            return;
        }
        fseek(vertexFile, 0L, SEEK_END);
        u32 vertexFileSize = ftell(vertexFile);
        u8* vertexCode = (u8*)PushArena(tempArena, vertexFileSize * sizeof(u8));
        rewind(vertexFile);
        fread(vertexCode, vertexFileSize * sizeof(u8), 1, vertexFile);
        fclose(vertexFile);

        VkShaderModuleCreateInfo vertexModuleInfo = {};
        vertexModuleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        vertexModuleInfo.codeSize = vertexFileSize;
        vertexModuleInfo.pCode = (u32*)vertexCode;

        VkShaderModule vertexModule = VK_NULL_HANDLE;
        vkCreateShaderModule(R_Device.handle, &vertexModuleInfo, nullptr, &vertexModule);

        // --AlNov: Fragment Shader
        const char* fragmentPath = "data/shaders/triangleFS.spv";
        FILE* fragmentFile = fopen(fragmentPath, "rb");
        if (!fragmentFile)
        {
            printf("Cannot open file %s\n", fragmentPath);
            return;
        }
        fseek(fragmentFile, 0L, SEEK_END);
        u32 fragmentFileSize = ftell(fragmentFile);
        u8* fragmentCode = (u8*)PushArena(tempArena, fragmentFileSize * sizeof(u8));
        rewind(fragmentFile);
        fread(fragmentCode, fragmentFileSize * sizeof(u8), 1, fragmentFile);
        fclose(fragmentFile);

        VkShaderModuleCreateInfo fragmentModuleInfo = {};
        fragmentModuleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        fragmentModuleInfo.codeSize = fragmentFileSize;
        fragmentModuleInfo.pCode = (u32*)fragmentCode;

        VkShaderModule fragmentModule = VK_NULL_HANDLE;
        vkCreateShaderModule(R_Device.handle, &fragmentModuleInfo, nullptr, &fragmentModule);

        VkPipelineShaderStageCreateInfo vertexShaderInfo = {};
        vertexShaderInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertexShaderInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertexShaderInfo.module = vertexModule;
        vertexShaderInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragmentShaderInfo = {};
        fragmentShaderInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragmentShaderInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragmentShaderInfo.module = fragmentModule;
        fragmentShaderInfo.pName = "main";

        VkPipelineShaderStageCreateInfo stages[] = {
            vertexShaderInfo,
            fragmentShaderInfo,
        };

        VkVertexInputBindingDescription vertexDescription = {};
        vertexDescription.binding = 0;
        vertexDescription.stride = sizeof(Vec3f);
        vertexDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        VkVertexInputAttributeDescription vertexAttributeDescription = {};
        vertexAttributeDescription.location = 0;
        vertexAttributeDescription.binding = 0;
        vertexAttributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
        // vertexAttributeDescription.offset = sizeof(Vec3f);
        vertexAttributeDescription.offset = 0;
        
        VkPipelineVertexInputStateCreateInfo vertexInputStateInfo = {};
        vertexInputStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputStateInfo.vertexBindingDescriptionCount = 1;
        vertexInputStateInfo.pVertexBindingDescriptions = &vertexDescription;
        vertexInputStateInfo.vertexAttributeDescriptionCount = 1;
        vertexInputStateInfo.pVertexAttributeDescriptions = &vertexAttributeDescription;

        VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateInfo = {};
        inputAssemblyStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblyStateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssemblyStateInfo.primitiveRestartEnable = VK_FALSE;

        // --AlNov: Viewport State
        VkViewport viewport = {};
        viewport.x = 0;
        viewport.y = 0;
        viewport.height = height;
        viewport.width = width;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor = {};
        scissor.offset = {0, 0};
        scissor.extent.height = height;
        scissor.extent.width = width;

        VkPipelineViewportStateCreateInfo viewportStateInfo = {};
        viewportStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportStateInfo.viewportCount = 1;
        viewportStateInfo.pViewports = &viewport;
        viewportStateInfo.scissorCount = 1;
        viewportStateInfo.pScissors = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterizationStateInfo = {};
        rasterizationStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizationStateInfo.depthClampEnable = VK_FALSE;
        rasterizationStateInfo.rasterizerDiscardEnable = VK_FALSE;
        rasterizationStateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizationStateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizationStateInfo.depthBiasEnable = VK_FALSE;
        rasterizationStateInfo.lineWidth = 1.0f;

        VkPipelineMultisampleStateCreateInfo multisampleStateInfo = {};
        multisampleStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampleStateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampleStateInfo.sampleShadingEnable = VK_FALSE;
        multisampleStateInfo.minSampleShading = 0.0f;
        multisampleStateInfo.pSampleMask = nullptr;
        multisampleStateInfo.alphaToCoverageEnable = VK_FALSE;
        multisampleStateInfo.alphaToOneEnable = VK_FALSE;

        VkPipelineDepthStencilStateCreateInfo depthStencilStateInfo = {};
        depthStencilStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencilStateInfo.depthTestEnable = VK_FALSE;
        depthStencilStateInfo.depthWriteEnable = VK_FALSE;
        depthStencilStateInfo.depthBoundsTestEnable = VK_FALSE;
        depthStencilStateInfo.stencilTestEnable = VK_FALSE;

        VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT| VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT
            | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorBlendStateInfo = {};
        colorBlendStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlendStateInfo.logicOpEnable = VK_FALSE;
        colorBlendStateInfo.attachmentCount = 1;
        colorBlendStateInfo.pAttachments = &colorBlendAttachment;

        VkPipelineLayoutCreateInfo layoutInfo = {};
        layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutInfo.setLayoutCount = 1;
        layoutInfo.pSetLayouts = &R_MVPDescriptor.layout;
        layoutInfo.pushConstantRangeCount = 0;
        layoutInfo.pPushConstantRanges = nullptr;

        vkCreatePipelineLayout(R_Device.handle, &layoutInfo, nullptr, &R_Pipeline.layout);

        // Color Attachment
        VkAttachmentDescription colorAttachment = {};
        colorAttachment.format = R_Swapchain.surfaceFormat.format;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentReference = {};
        colorAttachmentReference.attachment = 0;
        colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.inputAttachmentCount = 0;
        subpass.pInputAttachments = nullptr;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentReference;
        subpass.pResolveAttachments = nullptr;
        subpass.pDepthStencilAttachment = nullptr;
        subpass.preserveAttachmentCount = 0;
        subpass.pPreserveAttachments = nullptr;

        VkSubpassDependency subpassDependency = {};
        subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        subpassDependency.dstSubpass = 0;
        subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassDependency.srcAccessMask = 0;
        subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &subpassDependency;

        vkCreateRenderPass(R_Device.handle, &renderPassInfo, nullptr, &R_Pipeline.renderPass);

        VkGraphicsPipelineCreateInfo pipelineInfo = {};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = stages;
        pipelineInfo.pVertexInputState = &vertexInputStateInfo;
        pipelineInfo.pInputAssemblyState = &inputAssemblyStateInfo;
        pipelineInfo.pViewportState = &viewportStateInfo;
        pipelineInfo.pRasterizationState = &rasterizationStateInfo;
        pipelineInfo.pMultisampleState = &multisampleStateInfo;
        pipelineInfo.pDepthStencilState = &depthStencilStateInfo;
        pipelineInfo.pColorBlendState = &colorBlendStateInfo;
        pipelineInfo.pDynamicState = nullptr;
        pipelineInfo.layout = R_Pipeline.layout;
        pipelineInfo.renderPass = R_Pipeline.renderPass;
        pipelineInfo.subpass = 0;

        vkCreateGraphicsPipelines(R_Device.handle, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &R_Pipeline.handle);
    }
    FreeArena(tempArena);
}

void R_VK_CreateFramebuffers()
{
    R_Swapchain.framebuffers = (VkFramebuffer*)PushArena(R_Arena, R_Swapchain.imageCount * sizeof(VkFramebuffer));

    for (u32 i = 0; i < R_Swapchain.imageCount; ++i)
    {
        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = R_Pipeline.renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = &R_Swapchain.imageViews[i];
        framebufferInfo.width = R_Swapchain.extent.width;
        framebufferInfo.height = R_Swapchain.extent.height;
        framebufferInfo.layers = 1;

        vkCreateFramebuffer(R_Device.handle, &framebufferInfo, nullptr, &R_Swapchain.framebuffers[i]);
    }
}

void R_VK_CreateCommandPool()
{
    VkCommandPoolCreateInfo cmdPoolInfo = {};
    cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    cmdPoolInfo.queueFamilyIndex = R_Device.queueIndex;

    vkCreateCommandPool(R_Device.handle, &cmdPoolInfo, nullptr, &R_CmdPool.handle);
}

void R_VK_CreateMVPBuffer()
{
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof(R_MVP);
    bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkCreateBuffer(R_Device.handle, &bufferInfo, nullptr, &R_MVPBuffer);

    VkMemoryRequirements memoryRequirements = {};
    vkGetBufferMemoryRequirements(R_Device.handle, R_MVPBuffer, &memoryRequirements);

    VkMemoryPropertyFlags memoryProperties = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT|VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    VkPhysicalDeviceMemoryProperties tempProperties = {};
    vkGetPhysicalDeviceMemoryProperties(R_Device.GPU, &tempProperties);

    i32 memoryTypeIndex = -1;
    for (u32 typeIndex = 0; typeIndex < tempProperties.memoryTypeCount; ++typeIndex)
    {
        if (memoryRequirements.memoryTypeBits & (1 << typeIndex)
            && ((tempProperties.memoryTypes[typeIndex].propertyFlags & memoryProperties) == memoryProperties)
        )
        {
            memoryTypeIndex = typeIndex;
            break;
        }
    }

    VkMemoryAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.allocationSize = memoryRequirements.size;
    allocateInfo.memoryTypeIndex = memoryTypeIndex;
    vkAllocateMemory(R_Device.handle, &allocateInfo, nullptr, &R_MVPMemory);

    vkBindBufferMemory(R_Device.handle, R_MVPBuffer, R_MVPMemory, 0);
}

void R_VK_AllocateCommandBuffers()
{
    VkCommandBufferAllocateInfo cmdBufferInfo = {};
    cmdBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdBufferInfo.commandPool = R_CmdPool.handle;
    cmdBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdBufferInfo.commandBufferCount = CountArrayElements(R_CmdPool.cmdBuffers);

    vkAllocateCommandBuffers(R_Device.handle, &cmdBufferInfo, R_CmdPool.cmdBuffers);
}

void R_VK_CreateSyncTools()
{
    for (i32 i = 0; i < NUM_FRAMES_IN_FLIGHT; ++i)
    {
        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        vkCreateSemaphore(R_Device.handle, &semaphoreInfo, nullptr, &R_SyncTools.imageIsAvailableSemaphores[i]);
        vkCreateSemaphore(R_Device.handle, &semaphoreInfo, nullptr, &R_SyncTools.imageIsReadySemaphores[i]);

        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        vkCreateFence(R_Device.handle, &fenceInfo, nullptr, &R_SyncTools.fences[i]);
    }
}

void R_VK_CreateBuffer(VkBufferUsageFlags usage, VkMemoryPropertyFlags propertyFlags, u32 size, VkBuffer* outBuffer, VkDeviceMemory* outMemory)
{
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkCreateBuffer(R_Device.handle, &bufferInfo, nullptr, outBuffer);

    VkMemoryRequirements memoryRequirements = {};
    vkGetBufferMemoryRequirements(R_Device.handle, *outBuffer, &memoryRequirements);

    VkPhysicalDeviceMemoryProperties tempProperties = {};
    vkGetPhysicalDeviceMemoryProperties(R_Device.GPU, &tempProperties);

    i32 memoryTypeIndex = -1;
    for (u32 typeIndex = 0; typeIndex < tempProperties.memoryTypeCount; ++typeIndex)
    {
        if (memoryRequirements.memoryTypeBits & (1 << typeIndex)
            && ((tempProperties.memoryTypes[typeIndex].propertyFlags & propertyFlags) == propertyFlags)
        )
        {
            memoryTypeIndex = typeIndex;
            break;
        }
    }

    VkMemoryAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.allocationSize = memoryRequirements.size;
    allocateInfo.memoryTypeIndex = memoryTypeIndex;
    vkAllocateMemory(R_Device.handle, &allocateInfo, nullptr, outMemory);

    vkBindBufferMemory(R_Device.handle, *outBuffer, *outMemory, 0);
}

void R_VK_CopyToMemory(VkDeviceMemory memory, void* data, u32 size)
{
    void* mappedMemory;
    vkMapMemory(R_Device.handle, memory, 0, size, 0, &mappedMemory);
    {
        memcpy(mappedMemory, data, size);
    }
    vkUnmapMemory(R_Device.handle, memory);
}

void R_RecordCmdBuffer(VkCommandBuffer cmdBuffer, u32 imageIndex)
{
    vkResetCommandBuffer(cmdBuffer, 0);

    VkCommandBufferBeginInfo cmdBeginInfo = {};
    cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    
    vkBeginCommandBuffer(cmdBuffer, &cmdBeginInfo);
    {
        VkClearValue colorClearValue = {};
        colorClearValue.color = { {0.05f, 0.05f, 0.05f, 1.0f} };

        VkRenderPassBeginInfo renderPassBeginInfo = {};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = R_Pipeline.renderPass;
        renderPassBeginInfo.framebuffer = R_Swapchain.framebuffers[imageIndex];
        renderPassBeginInfo.renderArea.extent = R_Swapchain.extent;
        renderPassBeginInfo.renderArea.offset.y = 0;
        renderPassBeginInfo.renderArea.offset.y = 0;
        renderPassBeginInfo.clearValueCount = 1;
        renderPassBeginInfo.pClearValues = &colorClearValue;

        vkCmdBeginRenderPass(cmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        {
            vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, R_Pipeline.handle);

            vkCmdBindDescriptorSets(
                cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, R_Pipeline.layout,
                0, 1, &R_MVPDescriptor.handle, 0, 0
            );

            vkCmdDraw(cmdBuffer, 3, 1, 0, 0);
        }
        vkCmdEndRenderPass(cmdBuffer);
    }
    vkEndCommandBuffer(cmdBuffer);
}

void R_Draw(f32 deltaTime)
{
    localPersist u32 currentFrame = 0;
    // static u32 currentFrame = 0;

    vkWaitForFences(R_Device.handle, 1, &R_SyncTools.fences[currentFrame], VK_TRUE, U64_MAX);
    
    vkResetFences(R_Device.handle, 1, &R_SyncTools.fences[currentFrame]);

    // --AlNov: @TODO Read more about vkAcquireNextImageKHR in terms of synchonization
    u32 imageIndex;
    vkAcquireNextImageKHR(
        R_Device.handle, R_Swapchain.handle,
        U64_MAX, R_SyncTools.imageIsAvailableSemaphores[currentFrame],
        nullptr, &imageIndex
    );

    R_RecordCmdBuffer(R_CmdPool.cmdBuffers[currentFrame], imageIndex);

    VkPipelineStageFlags waitStage = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    };

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &R_SyncTools.imageIsAvailableSemaphores[imageIndex];
    submitInfo.pWaitDstStageMask = &waitStage;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &R_CmdPool.cmdBuffers[currentFrame];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &R_SyncTools.imageIsReadySemaphores[currentFrame];

    VkQueue queue;
    vkGetDeviceQueue(R_Device.handle, R_Device.queueIndex, 0, &queue);

    vkQueueSubmit(queue, 1, &submitInfo, R_SyncTools.fences[currentFrame]);

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &R_SyncTools.imageIsReadySemaphores[currentFrame];
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &R_Swapchain.handle;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;

    vkQueuePresentKHR(queue, &presentInfo);

    currentFrame = (currentFrame + 1) % NUM_FRAMES_IN_FLIGHT;
}

void R_DrawSquare()
{
    // AlNov: TEMP CODE START (SHOULD NOT BE THERE)

    // AlNov: TEMP CODE END

    localPersist u32 currentFrame = 0;
    // static u32 currentFrame = 0;

    vkWaitForFences(R_Device.handle, 1, &R_SyncTools.fences[currentFrame], VK_TRUE, U64_MAX);
    
    vkResetFences(R_Device.handle, 1, &R_SyncTools.fences[currentFrame]);

    // --AlNov: @TODO Read more about vkAcquireNextImageKHR in terms of synchonization
    u32 imageIndex;
    vkAcquireNextImageKHR(
        R_Device.handle, R_Swapchain.handle,
        U64_MAX, R_SyncTools.imageIsAvailableSemaphores[currentFrame],
        nullptr, &imageIndex
    );

    // R_RecordCmdBuffer(R_CmdPool.cmdBuffers[currentFrame], imageIndex);
    VkCommandBuffer cmdBuffer = R_CmdPool.cmdBuffers[currentFrame];
    vkResetCommandBuffer(cmdBuffer, 0);

    VkCommandBufferBeginInfo cmdBeginInfo = {};
    cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    
    vkBeginCommandBuffer(cmdBuffer, &cmdBeginInfo);
    {
        VkClearValue colorClearValue = {};
        colorClearValue.color = { {0.05f, 0.05f, 0.05f, 1.0f} };

        VkRenderPassBeginInfo renderPassBeginInfo = {};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = R_Pipeline.renderPass;
        renderPassBeginInfo.framebuffer = R_Swapchain.framebuffers[imageIndex];
        renderPassBeginInfo.renderArea.extent = R_Swapchain.extent;
        renderPassBeginInfo.renderArea.offset.y = 0;
        renderPassBeginInfo.renderArea.offset.y = 0;
        renderPassBeginInfo.clearValueCount = 1;
        renderPassBeginInfo.pClearValues = &colorClearValue;

        vkCmdBeginRenderPass(cmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        {
            vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, R_Pipeline.handle);

            VkDeviceSize offsets[] = { 0 };
            R_Square* squareToDraw = squareList.firstSquare;
            while (squareToDraw)
            {
                // VertexBuffer
                R_VK_CreateBuffer(
                    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT|VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                    sizeof(squareToDraw->vertecies), &squareToDraw->vertexBuffer, &squareToDraw->vertexBufferMemory
                );
                R_VK_CopyToMemory(squareToDraw->vertexBufferMemory, &squareToDraw->vertecies, sizeof(squareToDraw->vertecies));

                // IndexBuffer
                R_VK_CreateBuffer(
                    VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT|VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                    sizeof(squareToDraw->indecies), &squareToDraw->indexBuffer, &squareToDraw->indexBufferMemory
                );
                R_VK_CopyToMemory(squareToDraw->indexBufferMemory, &squareToDraw->indecies, sizeof(squareToDraw->indecies));

                // MVP BUffer
                R_VK_CreateBuffer(
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT|VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                    sizeof(squareToDraw->mvp), &squareToDraw->mvpBuffer, &squareToDraw->mvpBufferMemory
                );
                R_VK_CopyToMemory(squareToDraw->mvpBufferMemory, &squareToDraw->mvp, sizeof(squareToDraw->mvp));

                VkDescriptorSetAllocateInfo setInfo = {};
                setInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                setInfo.descriptorPool = R_DescriptorPool.handle;
                setInfo.descriptorSetCount = 1;
                setInfo.pSetLayouts = &R_MVPDescriptor.layout;

                vkAllocateDescriptorSets(R_Device.handle, &setInfo, &squareToDraw->mvpSet);

                VkDescriptorBufferInfo bufferInfo = {};
                bufferInfo.buffer = squareToDraw->mvpBuffer;
                bufferInfo.offset = 0;
                bufferInfo.range = sizeof(squareToDraw->mvp);

                VkWriteDescriptorSet writeSet = {};
                writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                writeSet.dstSet = squareToDraw->mvpSet;
                writeSet.dstBinding = 0;
                writeSet.dstArrayElement = 0;
                writeSet.descriptorCount = 1;
                writeSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                writeSet.pBufferInfo = &bufferInfo;

                vkUpdateDescriptorSets(R_Device.handle, 1, &writeSet, 0, nullptr);

                vkCmdBindDescriptorSets(
                    cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, R_Pipeline.layout,
                    0, 1, &squareToDraw->mvpSet, 0, 0
                );

                vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &squareToDraw->vertexBuffer, offsets);
                vkCmdBindIndexBuffer(cmdBuffer, squareToDraw->indexBuffer, 0, VK_INDEX_TYPE_UINT32);

                vkCmdDrawIndexed(cmdBuffer, 6, 1, 0, 0, 0);

                squareToDraw = squareToDraw->next;
            }
        }
        vkCmdEndRenderPass(cmdBuffer);
    }
    vkEndCommandBuffer(cmdBuffer);

    VkPipelineStageFlags waitStage = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    };

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &R_SyncTools.imageIsAvailableSemaphores[imageIndex];
    submitInfo.pWaitDstStageMask = &waitStage;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &R_CmdPool.cmdBuffers[currentFrame];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &R_SyncTools.imageIsReadySemaphores[currentFrame];

    VkQueue queue;
    vkGetDeviceQueue(R_Device.handle, R_Device.queueIndex, 0, &queue);

    vkQueueSubmit(queue, 1, &submitInfo, R_SyncTools.fences[currentFrame]);

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &R_SyncTools.imageIsReadySemaphores[currentFrame];
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &R_Swapchain.handle;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;

    vkQueuePresentKHR(queue, &presentInfo);

    currentFrame = (currentFrame + 1) % NUM_FRAMES_IN_FLIGHT;

    R_Square* squareToDraw = squareList.firstSquare;
    while (squareToDraw)
    {
        // --AlNov: @NOTE It is bad to recreate and delete.
        // But it is how it is now
        vkDeviceWaitIdle(R_Device.handle);

        vkDestroyBuffer(R_Device.handle, squareToDraw->vertexBuffer, nullptr);
        vkDestroyBuffer(R_Device.handle, squareToDraw->indexBuffer, nullptr);
        vkDestroyBuffer(R_Device.handle, squareToDraw->mvpBuffer, nullptr);

        vkFreeMemory(R_Device.handle, squareToDraw->vertexBufferMemory, nullptr);
        vkFreeMemory(R_Device.handle, squareToDraw->indexBufferMemory, nullptr);
        vkFreeMemory(R_Device.handle, squareToDraw->mvpBufferMemory, nullptr);

        vkResetDescriptorPool(R_Device.handle, R_DescriptorPool.handle, 0);

        squareToDraw = squareToDraw->next;
    }

    squareList = {};
}

void R_Init(const OS_Window& window)
{
    R_VK_CreateInstance();
    R_VK_CreateDevice();
    R_VK_CreateSwapchain(window);
    // --AlNov: @NOTE Maybe extent should be used for CreatePipeline
    R_VK_CreateMVPBuffer();
    R_VK_CreateDescriptorPool();
    R_VK_AllocateDesciptorSet();
    R_VK_CreatePipeline(window.width, window.height);
    R_VK_CreateFramebuffers();
    R_VK_CreateCommandPool();
    R_VK_AllocateCommandBuffers();
    R_VK_CreateSyncTools();
}

void R_VK_CreateDescriptorPool()
{
    u32 descriptorCount = 10;

    VkDescriptorPoolSize poolSize = {};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = descriptorCount;

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.maxSets = descriptorCount;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;

    vkCreateDescriptorPool(R_Device.handle, &poolInfo, nullptr, &R_DescriptorPool.handle);
}

void R_VK_AllocateDesciptorSet()
{
    VkDescriptorSetLayoutBinding bindingInfo = {};
    bindingInfo.binding = 0;
    bindingInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bindingInfo.descriptorCount = 1;
    bindingInfo.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &bindingInfo;

    vkCreateDescriptorSetLayout(R_Device.handle, &layoutInfo, nullptr, &R_MVPDescriptor.layout);

    // VkDescriptorSetAllocateInfo setInfo = {};
    // setInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    // setInfo.descriptorPool = R_DescriptorPool.handle;
    // setInfo.descriptorSetCount = 1;
    // setInfo.pSetLayouts = &R_MVPDescriptor.layout;

    // vkAllocateDescriptorSets(R_Device.handle, &setInfo, &R_MVPDescriptor.handle);

    // VkDescriptorBufferInfo bufferInfo = {};
    // bufferInfo.buffer = R_MVPBuffer;
    // bufferInfo.offset = 0;
    // bufferInfo.range = sizeof(R_MVP);

    // VkWriteDescriptorSet writeSet = {};
    // writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    // writeSet.dstSet = R_MVPDescriptor.handle;
    // writeSet.dstBinding = 0;
    // writeSet.dstArrayElement = 0;
    // writeSet.descriptorCount = 1;
    // writeSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    // writeSet.pBufferInfo = &bufferInfo;

    // vkUpdateDescriptorSets(R_Device.handle, 1, &writeSet, 0, nullptr);
}

void R_VK_CreateSwapchain(const OS_Window& window)
{
    Arena* tempArena = AllocateArena(Kilobytes(4));
    {
        VkWin32SurfaceCreateInfoKHR surfaceInfo = {};
        surfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        surfaceInfo.hinstance = window.instance;
        surfaceInfo.hwnd = window.handle;

        vkCreateWin32SurfaceKHR(R_VK_Instance, &surfaceInfo, nullptr, &R_Swapchain.surface);

        // Get Surface Capabilities
        VkSurfaceCapabilitiesKHR capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(R_Device.GPU, R_Swapchain.surface, &capabilities);

        R_Swapchain.extent = capabilities.currentExtent;

        R_Swapchain.imageCount = capabilities.minImageCount + 1;
        if (capabilities.maxImageCount > 0 && R_Swapchain.imageCount > capabilities.maxImageCount) {
            R_Swapchain.imageCount = capabilities.maxImageCount;
        }

        // Get Surface Format
        u32 formatCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(R_Device.GPU, R_Swapchain.surface, &formatCount, nullptr);
        VkSurfaceFormatKHR* formats = (VkSurfaceFormatKHR*)PushArena(tempArena, formatCount * sizeof(VkSurfaceFormatKHR));
        vkGetPhysicalDeviceSurfaceFormatsKHR(R_Device.GPU, R_Swapchain.surface, &formatCount, formats);

        for (u32 i = 0; i < formatCount; ++i)
        {
            if (formats[i].format == VK_FORMAT_B8G8R8A8_SRGB && formats[i].colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR) {
                R_Swapchain.surfaceFormat = formats[i];
            }
        }

        VkSwapchainCreateInfoKHR swapchainInfo = {};
        swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchainInfo.surface = R_Swapchain.surface;
        swapchainInfo.minImageCount = R_Swapchain.imageCount;
        swapchainInfo.imageFormat = R_Swapchain.surfaceFormat.format;
        swapchainInfo.imageColorSpace = R_Swapchain.surfaceFormat.colorSpace;
        swapchainInfo.imageExtent = capabilities.currentExtent;
        swapchainInfo.imageArrayLayers = 1;
        swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchainInfo.queueFamilyIndexCount = 0;
        swapchainInfo.pQueueFamilyIndices = nullptr;
        swapchainInfo.preTransform = capabilities.currentTransform;
        swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapchainInfo.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
        swapchainInfo.clipped = VK_TRUE;
        swapchainInfo.oldSwapchain = VK_NULL_HANDLE;

        vkCreateSwapchainKHR(R_Device.handle, &swapchainInfo, nullptr, &R_Swapchain.handle);

        vkGetSwapchainImagesKHR(R_Device.handle, R_Swapchain.handle, &R_Swapchain.imageCount, nullptr);
        R_Swapchain.images = (VkImage*)PushArena(R_Arena, R_Swapchain.imageCount * sizeof(VkImage));
        vkGetSwapchainImagesKHR(R_Device.handle, R_Swapchain.handle, &R_Swapchain.imageCount, R_Swapchain.images);

        R_Swapchain.imageViews = (VkImageView*)PushArena(R_Arena, R_Swapchain.imageCount * sizeof(VkImageView));
        for (u32 i = 0; i < R_Swapchain.imageCount; ++i)
        {
            VkImageViewCreateInfo imageViewInfo = {};
            imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            imageViewInfo.image = R_Swapchain.images[i];
            imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            imageViewInfo.format = R_Swapchain.surfaceFormat.format;
            imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageViewInfo.subresourceRange.baseMipLevel = 0;
            imageViewInfo.subresourceRange.levelCount = 1;
            imageViewInfo.subresourceRange.baseArrayLayer = 0;
            imageViewInfo.subresourceRange.layerCount = 1;

            vkCreateImageView(R_Device.handle, &imageViewInfo, nullptr, &R_Swapchain.imageViews[i]);
        }
    }
    FreeArena(tempArena);
}