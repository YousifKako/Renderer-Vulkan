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
    static constexpr uint32_t WIDTH             = 800;
    static constexpr uint32_t HEIGHT            = 600;

    Window window                               = Window(WIDTH, HEIGHT, "Renderer in Vulkan");
    Device device                               = Device(window);
    std::unique_ptr<SwapChain> swapChain        = nullptr;

    std::unique_ptr<Pipeline> pipeline          = nullptr;
    VkPipelineLayout pipelineLayout             = nullptr;
    std::vector<VkCommandBuffer> commandBuffers = { };

    std::unique_ptr<Model> model                = nullptr;

    void loadModels();
    void createPiplineLayout();
    void createPipeline();
    void createCommandBuffers();
    void freeCommandBuffers();
    void drawFrame();
    void recreateSwapChain();
    void recordCommandBuffer(int imageIndex);

public:
    Application();
    ~Application();

    // Delete copy constructor and copy operator
    Application(const Application&)            = delete;
    Application& operator=(const Application&) = delete;

    void run();
};