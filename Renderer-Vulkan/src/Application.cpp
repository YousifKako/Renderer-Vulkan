#include <stdexcept>
#include <array>

#include <Application.hpp>

Application::Application()
{
    this->load_models();
    this->create_pipline_layout();
    this->recreate_swap_chain();
    this->create_command_buffers();
}

Application::~Application()
{
    vkDestroyPipelineLayout(this->device.device(), this->pipeline_layout, nullptr);
}

void Application::run()
{
    while (!window.should_close())
    {
        glfwPollEvents();
        this->draw_frame();
    }

    vkDeviceWaitIdle(this->device.device());
}

void Application::load_models()
{
    std::vector<Model::Vertex> vectices
    {
        {{ 0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{ 0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}}
    };

    this->model = std::make_unique<Model>(this->device, vectices);
}

void Application::create_pipline_layout()
{
    VkPipelineLayoutCreateInfo pipeline_layout_info = VkPipelineLayoutCreateInfo();
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 0;
    pipeline_layout_info.pSetLayouts = nullptr;
    pipeline_layout_info.pushConstantRangeCount = 0;
    pipeline_layout_info.pPushConstantRanges = nullptr;

    if (vkCreatePipelineLayout(this->device.device(), &pipeline_layout_info, nullptr, &this->pipeline_layout) != VK_SUCCESS)
        throw std::runtime_error("Failed to Create Pipeline Layout!");
}

void Application::create_pipeline()
{
    assert(this->swap_chain != nullptr && "Cannot Create Pipeline Before Swap Chain");
    assert(this->pipeline_layout != nullptr && "Cannot Create Pipeline Before Layout");

    PipelineConfigInfo config = PipelineConfigInfo();
    Pipeline::default_pipeline_config(config);
    config.render_pass = this->swap_chain->getRenderPass();
    config.pipeline_layout = this->pipeline_layout;
    pipeline = std::make_unique<Pipeline>(this->device,
        "Assets/Shaders/Sample.vert.spv",
        "Assets/Shaders/Sample.frag.spv",
        config);
}

void Application::create_command_buffers()
{
    this->command_buffers.resize(this->swap_chain->imageCount());
    VkCommandBufferAllocateInfo allocation_info = VkCommandBufferAllocateInfo();
    allocation_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocation_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocation_info.commandPool = this->device.getCommandPool();
    allocation_info.commandBufferCount = static_cast<uint32_t>(this->command_buffers.size());

    if (vkAllocateCommandBuffers(this->device.device(), &allocation_info, this->command_buffers.data()) != VK_SUCCESS)
        throw std::runtime_error("Failed to Allocate Command Buffers!");

}

void Application::free_command_buffers()
{
    vkFreeCommandBuffers(this->device.device(), this->device.getCommandPool(), static_cast<uint32_t>(this->command_buffers.size()), this->command_buffers.data());
    this->command_buffers.clear();
}

void Application::draw_frame()
{
    uint32_t image_index = 0;
    auto result = this->swap_chain->acquireNextImage(&image_index);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        this->recreate_swap_chain();
        return;
    }

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        throw std::runtime_error("Failed to Acquire Swap Chain Image!");


    this->record_command_buffer(image_index);
    result = this->swap_chain->submitCommandBuffers(&this->command_buffers[image_index], &image_index);

    if (result == VK_ERROR_OUT_OF_DATE_KHR ||
        result == VK_SUBOPTIMAL_KHR        ||
        this->window.was_window_resized())
    {
        this->window.reset_window_resized_flag();
        this->recreate_swap_chain();
        return;
    }

    if (result != VK_SUCCESS)
        throw std::runtime_error("Failed to Present Swap Chain Image!");
}

void Application::recreate_swap_chain()
{
    auto extent = this->window.get_extent();

    while (extent.width == 0 || extent.height == 0)
    {
        extent = this->window.get_extent();
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(this->device.device());
    this->swap_chain = nullptr;
    if (this->swap_chain == nullptr)
        this->swap_chain = std::make_unique<SwapChain>(this->device, extent);
    else
    {
        this->swap_chain = std::make_unique<SwapChain>(this->device, extent, std::move(this->swap_chain));
        if (this->swap_chain->imageCount() != this->command_buffers.size())
        {
            this->free_command_buffers();
            this->create_command_buffers();
        }
    }
    this->create_pipeline();
}

void Application::record_command_buffer(int image_index)
{
    VkCommandBufferBeginInfo begin_info = VkCommandBufferBeginInfo();
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(this->command_buffers[image_index], &begin_info) != VK_SUCCESS)
        throw std::runtime_error("Failed to Begin Recording Command Buffer!");

    VkRenderPassBeginInfo render_pass_info = VkRenderPassBeginInfo();
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = this->swap_chain->getRenderPass();
    render_pass_info.framebuffer = this->swap_chain->getFrameBuffer(image_index);
    render_pass_info.renderArea.offset = { 0, 0 };
    render_pass_info.renderArea.extent = this->swap_chain->getSwapChainExtent();

    std::array<VkClearValue, 2> clear_values = { };
    clear_values[0].color = { 0.1f, 0.1f, 0.1f, 1.f };
    clear_values[1].depthStencil = { 1.0f, 0 };
    render_pass_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
    render_pass_info.pClearValues = clear_values.data();

    vkCmdBeginRenderPass(this->command_buffers[image_index], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(this->swap_chain->getSwapChainExtent().width);
    viewport.height = static_cast<float>(this->swap_chain->getSwapChainExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    VkRect2D scissor{ {0, 0}, this->swap_chain->getSwapChainExtent() };
    vkCmdSetViewport(this->command_buffers[image_index], 0, 1, &viewport);
    vkCmdSetScissor(this->command_buffers[image_index], 0, 1, &scissor);

    this->pipeline->bind(this->command_buffers[image_index]);
    this->model->bind(this->command_buffers[image_index]);
    this->model->draw(this->command_buffers[image_index]);

    vkCmdEndRenderPass(this->command_buffers[image_index]);
    if (vkEndCommandBuffer(this->command_buffers[image_index]) != VK_SUCCESS)
        throw std::runtime_error("Failed to Record Command Buffer!");
}