#pragma once

#include <memory>
#include <cassert>

#include <Window.hpp>
#include <Device.hpp>
#include <SwapChain.hpp>

class Renderer
{
private:
    Window& window;
    Device& device;
    std::unique_ptr<SwapChain> swapChain        = nullptr;
    std::vector<VkCommandBuffer> commandBuffers = { };
        
    uint32_t currentImageIndex                  = 0;
    uint32_t currentFrameIndex                  = 0;
    bool isFrameStarted                         = false;

    void createCommandBuffers();
    void freeCommandBuffers();
    void recreateSwapChain();

public:
    Renderer(Window& window, Device& device);
    ~Renderer();

    // Delete copy constructor and copy operator
    Renderer(const Renderer&)            = delete;
    Renderer& operator=(const Renderer&) = delete;

    VkCommandBuffer beginFrame();
    void endFrame();
    void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
    void endSwapChainRenderPass(VkCommandBuffer commandBuffer);
    VkRenderPass getSwapChainRenderPass() const;
    float getAspectRatio() const;
    bool isFrameInProgress() const;
    VkCommandBuffer getCurrentCommandBuffer() const;
    uint32_t getFrameIndex() const;
};