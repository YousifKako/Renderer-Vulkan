#include <stdexcept>
#include <array>

#include <Application.hpp>

Application::Application()
{
    this->load_models();
    this->create_pipline_layout();
    this->create_pipeline();
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
    auto pipeline_config = Pipeline::default_pipeline_config(this->swap_chain.width(), this->swap_chain.height());
    pipeline_config.render_pass = this->swap_chain.getRenderPass();
    pipeline_config.pipeline_layout = this->pipeline_layout;
    pipeline = std::make_unique<Pipeline>(this->device,
        "Assets/Shaders/Sample.vert.spv",
        "Assets/Shaders/Sample.frag.spv",
        pipeline_config);
}

void Application::create_command_buffers()
{
    this->command_buffer.resize(this->swap_chain.imageCount());
    VkCommandBufferAllocateInfo allocation_info = VkCommandBufferAllocateInfo();
    allocation_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocation_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocation_info.commandPool = this->device.getCommandPool();
    allocation_info.commandBufferCount = static_cast<uint32_t>(this->command_buffer.size());

    if (vkAllocateCommandBuffers(this->device.device(), &allocation_info, this->command_buffer.data()) != VK_SUCCESS)
        throw std::runtime_error("Failed to Allocate Command Buffers!");

    for (uint32_t i = 0; i < this->command_buffer.size(); ++i)
    {
        VkCommandBufferBeginInfo begin_info = VkCommandBufferBeginInfo();
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(this->command_buffer[i], &begin_info) != VK_SUCCESS)
            throw std::runtime_error("Failed to Begin Recording Command Buffer!");

        VkRenderPassBeginInfo render_pass_info = VkRenderPassBeginInfo();
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_info.renderPass = this->swap_chain.getRenderPass();
        render_pass_info.framebuffer = this->swap_chain.getFrameBuffer(i);
        render_pass_info.renderArea.offset = { 0, 0 };
        render_pass_info.renderArea.extent = this->swap_chain.getSwapChainExtent();

        std::array<VkClearValue, 2> clear_values = { };
        clear_values[0].color = { 0.1f, 0.1f, 0.1f, 1.f };
        clear_values[1].depthStencil = { 1.0f, 0 };
        render_pass_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
        render_pass_info.pClearValues = clear_values.data();

        vkCmdBeginRenderPass(this->command_buffer[i], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
        this->pipeline->bind(this->command_buffer[i]);
        this->model->bind(this->command_buffer[i]);
        this->model->draw(this->command_buffer[i]);

        vkCmdEndRenderPass(this->command_buffer[i]);
        if (vkEndCommandBuffer(this->command_buffer[i]) != VK_SUCCESS)
            throw std::runtime_error("Failed to Record Command Buffer!");
    }
}

void Application::draw_frame()
{
    uint32_t image_index = 0;
    auto result = this->swap_chain.acquireNextImage(&image_index);
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        throw std::runtime_error("Failed to Acquire Swap Chain Image!");

    result = this->swap_chain.submitCommandBuffers(&this->command_buffer[image_index], &image_index);
    if (result != VK_SUCCESS)
        throw std::runtime_error("Failed to Present Swap Chain Image!");
}