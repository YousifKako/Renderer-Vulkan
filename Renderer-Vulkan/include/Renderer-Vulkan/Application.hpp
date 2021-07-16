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

    Window window = Window(WIDTH, HEIGHT, "Raytracer in Vulkan");
    Device device = Device(window);
    SwapChain swap_chain = SwapChain(this->device, this->window.get_extent());

    std::unique_ptr<Pipeline> pipeline = nullptr;
    VkPipelineLayout pipeline_layout = nullptr;
    std::vector<VkCommandBuffer> command_buffer = { };

    std::unique_ptr<Model> model = nullptr;

    void load_models();
    void create_pipline_layout();
    void create_pipeline();
    void create_command_buffers();
    void draw_frame();

public:
    Application();
    ~Application();

    // Delete copy constructor and copy operator
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    void run();
};