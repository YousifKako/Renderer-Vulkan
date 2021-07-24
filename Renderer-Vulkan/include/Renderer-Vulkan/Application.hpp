#pragma once

#include <memory>

#include <Window.hpp>
#include <Pipeline.hpp>
#include <Device.hpp>
#include <SwapChain.hpp>
#include <Model.hpp>

class Application
{
private:
    static constexpr uint32_t WIDTH = 800;
    static constexpr uint32_t HEIGHT = 600;

    Window window = Window(WIDTH, HEIGHT, "Renderer in Vulkan");
    Device device = Device(window);
    std::unique_ptr<SwapChain> swap_chain = nullptr;

    std::unique_ptr<Pipeline> pipeline = nullptr;
    VkPipelineLayout pipeline_layout = nullptr;
    std::vector<VkCommandBuffer> command_buffers = { };

    std::unique_ptr<Model> model = nullptr;

    void load_models();
    void create_pipline_layout();
    void create_pipeline();
    void create_command_buffers();
    void free_command_buffers();
    void draw_frame();
    void recreate_swap_chain();
    void record_command_buffer(int image_index);

public:
    Application();
    ~Application();

    // Delete copy constructor and copy operator
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    void run();
};