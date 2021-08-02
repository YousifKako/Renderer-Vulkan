#pragma once

#include <memory>

#include <Pipeline.hpp>
#include <Device.hpp>
#include <Objects/Object.hpp>
#include <Camera.hpp>

class RenderSystem
{
private:
    Device& device;

    std::unique_ptr<Pipeline> pipeline = nullptr;
    VkPipelineLayout pipelineLayout    = nullptr;

    void createPiplineLayout();
    void createPipeline(VkRenderPass renderPass);

public:
    RenderSystem(Device& device, const VkRenderPass& renderPass);
    ~RenderSystem();

    // Delete copy constructor and copy operator
    RenderSystem(const RenderSystem&)            = delete;
    RenderSystem& operator=(const RenderSystem&) = delete;

    void renderObjects(VkCommandBuffer commandBuffer, std::vector<Object>& objects, const Camera& camera);
};