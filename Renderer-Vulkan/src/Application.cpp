#include <stdexcept>
#include <array>

#include <Application.hpp>

Application::Application()
{
    this->loadModels();
    this->createPiplineLayout();
    this->recreateSwapChain();
    this->createCommandBuffers();
}

Application::~Application()
{
    vkDestroyPipelineLayout(this->device.device(), this->pipelineLayout, nullptr);
}

void
Application::run()
{
    while (!window.shouldClose())
    {
        glfwPollEvents();
        this->drawFrame();
    }

    vkDeviceWaitIdle(this->device.device());
}

void
Application::loadModels()
{
    std::vector<Model::Vertex> vectices
    {
        {{ 0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{ 0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}}
    };

    this->model = std::make_unique<Model>(this->device, vectices);
}

void
Application::createPiplineLayout()
{
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = VkPipelineLayoutCreateInfo();
    pipelineLayoutInfo.sType                      = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount             = 0;
    pipelineLayoutInfo.pSetLayouts                = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount     = 0;
    pipelineLayoutInfo.pPushConstantRanges        = nullptr;

    if (vkCreatePipelineLayout(this->device.device(), &pipelineLayoutInfo, nullptr, &this->pipelineLayout) != VK_SUCCESS)
        throw std::runtime_error("Failed to Create Pipeline Layout!");
}

void
Application::createPipeline()
{
    assert(this->swapChain != nullptr && "Cannot Create Pipeline Before Swap Chain");
    assert(this->pipelineLayout != nullptr && "Cannot Create Pipeline Before Layout");

    PipelineConfigInfo config = PipelineConfigInfo();
    Pipeline::defaultPipelineConfig(config);
    config.renderPass         = this->swapChain->getRenderPass();
    config.pipelineLayout     = this->pipelineLayout;
    pipeline = std::make_unique<Pipeline>(this->device,
        "Assets/Shaders/Sample.vert.spv",
        "Assets/Shaders/Sample.frag.spv",
        config);
}

void
Application::createCommandBuffers()
{
    this->commandBuffers.resize(this->swapChain->imageCount());
    VkCommandBufferAllocateInfo allocationInfo = VkCommandBufferAllocateInfo();
    allocationInfo.sType                       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocationInfo.level                       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocationInfo.commandPool                 = this->device.getCommandPool();
    allocationInfo.commandBufferCount          = static_cast<uint32_t>(this->commandBuffers.size());

    if (vkAllocateCommandBuffers(this->device.device(), &allocationInfo, this->commandBuffers.data()) != VK_SUCCESS)
        throw std::runtime_error("Failed to Allocate Command Buffers!");
}

void
Application::freeCommandBuffers()
{
    vkFreeCommandBuffers(this->device.device(), this->device.getCommandPool(), static_cast<uint32_t>(this->commandBuffers.size()), this->commandBuffers.data());
    this->commandBuffers.clear();
}

void
Application::drawFrame()
{
    uint32_t imageIndex = 0;
    auto result = this->swapChain->acquireNextImage(&imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        this->recreateSwapChain();
        return;
    }

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        throw std::runtime_error("Failed to Acquire Swap Chain Image!");


    this->recordCommandBuffer(imageIndex);
    result = this->swapChain->submitCommandBuffers(&this->commandBuffers[imageIndex], &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR ||
        result == VK_SUBOPTIMAL_KHR        ||
        this->window.wasWindowResized())
    {
        this->window.resetWindowResizedFlag();
        this->recreateSwapChain();
        return;
    }

    if (result != VK_SUCCESS)
        throw std::runtime_error("Failed to Present Swap Chain Image!");
}

void
Application::recreateSwapChain()
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
        this->swapChain = std::make_unique<SwapChain>(this->device, extent, std::move(this->swapChain));
        if (this->swapChain->imageCount() != this->commandBuffers.size())
        {
            this->freeCommandBuffers();
            this->createCommandBuffers();
        }
    }
    this->createPipeline();
}

void
Application::recordCommandBuffer(int imageIndex)
{
    VkCommandBufferBeginInfo beginInfo = VkCommandBufferBeginInfo();
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(this->commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS)
        throw std::runtime_error("Failed to Begin Recording Command Buffer!");

    VkRenderPassBeginInfo renderPassInfo    = VkRenderPassBeginInfo();
    renderPassInfo.sType                    = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass               = this->swapChain->getRenderPass();
    renderPassInfo.framebuffer              = this->swapChain->getFrameBuffer(imageIndex);
    renderPassInfo.renderArea.offset        = { 0, 0 };
    renderPassInfo.renderArea.extent        = this->swapChain->getSwapChainExtent();

    std::array<VkClearValue, 2> clearValues = { };
    clearValues[0].color                    = { 0.1f, 0.1f, 0.1f, 1.f };
    clearValues[1].depthStencil             = { 1.0f, 0 };
    renderPassInfo.clearValueCount          = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues             = clearValues.data();

    vkCmdBeginRenderPass(this->commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport = { };
    viewport.x          = 0.0f;
    viewport.y          = 0.0f;
    viewport.width      = static_cast<float>(this->swapChain->getSwapChainExtent().width);
    viewport.height     = static_cast<float>(this->swapChain->getSwapChainExtent().height);
    viewport.minDepth   = 0.0f;
    viewport.maxDepth   = 1.0f;
    
    VkRect2D scissor    = { {0, 0}, this->swapChain->getSwapChainExtent() };

    vkCmdSetViewport(this->commandBuffers[imageIndex], 0, 1, &viewport);
    vkCmdSetScissor(this->commandBuffers[imageIndex], 0, 1, &scissor);

    this->pipeline->bind(this->commandBuffers[imageIndex]);
    this->model->bind(this->commandBuffers[imageIndex]);
    this->model->draw(this->commandBuffers[imageIndex]);

    vkCmdEndRenderPass(this->commandBuffers[imageIndex]);
    if (vkEndCommandBuffer(this->commandBuffers[imageIndex]) != VK_SUCCESS)
        throw std::runtime_error("Failed to Record Command Buffer!");
}