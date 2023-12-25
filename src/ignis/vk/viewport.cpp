#include "viewport.h"

#include "ignis/vk/instance.h"
#include "ignis/vk/device.h"
#include "ignis/vk/swapchain.h"
#include "ignis/window/win_window.h"
#include "ignis/vk/error.h"

Viewport::Viewport(const Instance& instance, const Device& device)
    : instance(instance)
    , device(device)
{
}

Viewport::~Viewport()
{
    for (VkFramebuffer& framebuffer : framebuffers)
    {
        vkDestroyFramebuffer(device.getHandle(), framebuffer, nullptr);
    }

    for (VkSemaphore& semaphore : renderFinishedSemaphores)
    {
        vkDestroySemaphore(device.getHandle(), semaphore, nullptr);
    }

    delete window;
    delete swapchain;
}

void Viewport::init(const char* windowName, uint32_t height, uint32_t width)
{
    window = new Window(windowName, height, width);
    swapchain = new Swapchain(instance,  device, window->instance, window->windowHandle);

    createSyncTools();
}

void Viewport::showWindow()
{
    window->show();
}

Swapchain* Viewport::getSwapchain() const
{
    return swapchain;
}

uint32_t Viewport::getImageCount() const
{
    return swapchain->imageCount;
}

std::vector<VkImageView> Viewport::getImageViews()
{
    return swapchain->imageViews; 
}

VkExtent2D Viewport::getExtent() const
{
    return swapchain->extent;
}

void Viewport::createFramebuffers(const VkRenderPass& renderPass)
{
    framebuffers.resize(swapchain->imageCount);

    for (int i = 0; i < framebuffers.size(); i++)
    {
		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = &swapchain->imageViews[i];

		VkExtent2D extent = swapchain->getExtent();
		framebufferInfo.width = extent.width;
		framebufferInfo.height = extent.height;
		framebufferInfo.layers = 1;

        VK_CHECK_ERROR(vkCreateFramebuffer(device.getHandle(), &framebufferInfo, nullptr, &framebuffers[i]), "Cannot create framebuffer");
    }
}

uint32_t Viewport::acquireNextImageIndex(const VkSemaphore& semaphore)
{
    return swapchain->acquireNextImage(semaphore);
}

void Viewport::present(uint32_t currentFrame, uint32_t imageIndex)
{
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &renderFinishedSemaphores[currentFrame];
	presentInfo.swapchainCount = 1;
	VkSwapchainKHR usedSpawchain = swapchain->getVkSwapchain();
	presentInfo.pSwapchains = &usedSpawchain;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;

    VkQueue queue;
    device.getGraphicsQueue(&queue);

    vkQueuePresentKHR(queue, &presentInfo);
}

void Viewport::createSyncTools()
{
    renderFinishedSemaphores.resize(swapchain->imageCount);

    for (int i = 0; i < renderFinishedSemaphores.size(); i++)
    {
        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        vkCreateSemaphore(device.getHandle(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]);
    }
}
