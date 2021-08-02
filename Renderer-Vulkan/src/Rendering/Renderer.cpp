#include <stdexcept>
#include <array>

#include <Rendering/Renderer.hpp>


Renderer::Renderer(Window& window, Device& device)
    : window(window), device(device)
{
    this->recreateSwapChain();
    this->createCommandBuffers();
}

Renderer::~Renderer()
{
    freeCommandBuffers();
}

void
Renderer::createCommandBuffers()
{
    this->commandBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
    VkCommandBufferAllocateInfo allocationInfo = VkCommandBufferAllocateInfo();
    allocationInfo.sType                       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocationInfo.level                       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocationInfo.commandPool                 = this->device.getCommandPool();
    allocationInfo.commandBufferCount          = static_cast<uint32_t>(this->commandBuffers.size());

    if (vkAllocateCommandBuffers(this->device.device(), &allocationInfo, this->commandBuffers.data()) != VK_SUCCESS)
        throw std::runtime_error("Failed to Allocate Command Buffers!");
}

void
Renderer::freeCommandBuffers()
{
    vkFreeCommandBuffers(this->device.device(), this->device.getCommandPool(), static_cast<uint32_t>(this->commandBuffers.size()), this->commandBuffers.data());
    this->commandBuffers.clear();
}

void
Renderer::recreateSwapChain()
{
    auto extent = this->window.getExtent();

    while (extent.width == 0 || extent.height == 0)
    {
        extent = this->window.getExtent();
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(this->device.device());
    if (this->swapChain == nullptr)
        this->swapChain = std::make_unique<SwapChain>(this->device, extent);
    else
    {
        std::shared_ptr<SwapChain> oldSwapChain = std::move(this->swapChain);
        this->swapChain = std::make_unique<SwapChain>(this->device, extent, oldSwapChain);

        if (!oldSwapChain->compareSwapFormat(*this->swapChain.get()))
            throw std::runtime_error("Swap Chain Image (or Depth) Format has Changed");
    }
}

VkCommandBuffer
Renderer::beginFrame()
{
    assert(!this->isFrameStarted && "Can't Call beginFrame While Already in Progress");
    
    auto result = this->swapChain->acquireNextImage(&this->currentImageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        this->recreateSwapChain();
        return nullptr;
    }

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        throw std::runtime_error("Failed to Acquire Swap Chain Image!");

    this->isFrameStarted               = true;

    auto commandBuffer                 = this->getCurrentCommandBuffer();
    VkCommandBufferBeginInfo beginInfo = VkCommandBufferBeginInfo();
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
        throw std::runtime_error("Failed to Begin Recording Command Buffer!");

    return commandBuffer;
}

void
Renderer::endFrame()
{
    assert(this->isFrameStarted && "Can't Call endFrame While Frame is not in Progress");
    auto commandBuffer = this->getCurrentCommandBuffer();

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
        throw std::runtime_error("Failed to Record Command Buffer!");

    auto result = this->swapChain->submitCommandBuffers(&commandBuffer, &this->currentImageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR ||
        result == VK_SUBOPTIMAL_KHR ||
        this->window.wasWindowResized())
    {
        this->window.resetWindowResizedFlag();
        this->recreateSwapChain();
    }
    else if (result != VK_SUCCESS)
        throw std::runtime_error("Failed to Present Swap Chain Image!");

    this->isFrameStarted    = false;
    this->currentFrameIndex = (this->currentFrameIndex + 1) % SwapChain::MAX_FRAMES_IN_FLIGHT;
}

void
Renderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer)
{
    assert(this->isFrameStarted && "Can't Call beginSwapChainRenderPass if Frame is not in Progress");
    assert(commandBuffer == this->getCurrentCommandBuffer() &&
        "Can't Begin Render Pass on Command Buffer from a Different Frame");

    VkRenderPassBeginInfo renderPassInfo    = VkRenderPassBeginInfo();
    renderPassInfo.sType                    = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass               = this->swapChain->getRenderPass();
    renderPassInfo.framebuffer              = this->swapChain->getFrameBuffer(this->currentImageIndex);
    renderPassInfo.renderArea.offset        = { 0, 0 };
    renderPassInfo.renderArea.extent        = this->swapChain->getSwapChainExtent();

    std::array<VkClearValue, 2> clearValues = { };
    clearValues[0].color                    = { 0.01f, 0.01f, 0.01f, 1.f };
    clearValues[1].depthStencil             = { 1.0f, 0 };
    renderPassInfo.clearValueCount          = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues             = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport = { };
    viewport.x          = 0.0f;
    viewport.y          = 0.0f;
    viewport.width      = static_cast<float>(this->swapChain->getSwapChainExtent().width);
    viewport.height     = static_cast<float>(this->swapChain->getSwapChainExtent().height);
    viewport.minDepth   = 0.0f;
    viewport.maxDepth   = 1.0f;

    VkRect2D scissor    = { { 0, 0 }, this->swapChain->getSwapChainExtent() };

    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void
Renderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer)
{
    assert(this->isFrameStarted && "Can't Call endSwapChainRenderPass if Frame is not in Progress");
    assert(commandBuffer == this->getCurrentCommandBuffer() &&
        "Can't End Render Pass on Command Buffer from a Different Frame");

    vkCmdEndRenderPass(commandBuffer);
}

VkRenderPass
Renderer::getSwapChainRenderPass() const
{
    return this->swapChain->getRenderPass();
}

float
Renderer::getAspectRatio() const
{
    return this->swapChain->extentAspectRatio();
}

bool
Renderer::isFrameInProgress() const
{
    return this->isFrameStarted;
}

VkCommandBuffer
Renderer::getCurrentCommandBuffer() const
{
    assert(this->isFrameStarted && "Cannot Get Command Buffer when Frame not in Progress");

    return this->commandBuffers[this->currentFrameIndex];
}

uint32_t Renderer::getFrameIndex() const
{
    assert(this->isFrameStarted && "Cannot Get Frame Index when Frame not in Progress");
    return this->currentFrameIndex;
}