#include <stdexcept>
#include <array>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <Rendering/RenderSystem.hpp>

struct PushConstantData
{
    glm::mat4 transform = { };
    alignas(16) glm::vec3 color;
};

RenderSystem::RenderSystem(Device& device, const VkRenderPass& renderPass)
    : device(device)
{
    this->createPiplineLayout();
    this->createPipeline(renderPass);
}

RenderSystem::~RenderSystem()
{
    vkDestroyPipelineLayout(this->device.device(), this->pipelineLayout, nullptr);
}

void
RenderSystem::createPiplineLayout()
{
    VkPushConstantRange pushConstantRange         = { };
    pushConstantRange.stageFlags                  =
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset                      = 0;
    pushConstantRange.size                        = sizeof(PushConstantData);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = VkPipelineLayoutCreateInfo();
    pipelineLayoutInfo.sType                      = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount             = 0;
    pipelineLayoutInfo.pSetLayouts                = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount     = 1;
    pipelineLayoutInfo.pPushConstantRanges        = &pushConstantRange;

    if (vkCreatePipelineLayout(this->device.device(), &pipelineLayoutInfo, nullptr, &this->pipelineLayout) != VK_SUCCESS)
        throw std::runtime_error("Failed to Create Pipeline Layout!");
}

void
RenderSystem::createPipeline(VkRenderPass renderPass)
{
    assert(this->pipelineLayout != nullptr && "Cannot Create Pipeline Before Layout");

    PipelineConfigInfo config = PipelineConfigInfo();
    Pipeline::defaultPipelineConfig(config);
    config.renderPass         = renderPass;
    config.pipelineLayout     = this->pipelineLayout;
    this->pipeline            = std::make_unique<Pipeline>(this->device,
                                       "Assets/Shaders/Vertex.vert.spv",
                                       "Assets/Shaders/Fragment.frag.spv",
                                       config);
}

void
RenderSystem::renderObjects(VkCommandBuffer commandBuffer, std::vector<Object>& objects, const Camera& camera)
{
    this->pipeline->bind(commandBuffer);

    // TODO: Send `projectionView` Calculation to the GPU instead of CPU
    auto projectionView = camera.getProjection() * camera.getView();

    for (auto& object : objects)
    {
        PushConstantData push = { };
        push.color            = object.color;

        // TODO: Send `transform` Calculation to the GPU instead of CPU
        push.transform        = projectionView * object.transform.mat4();

        vkCmdPushConstants(
            commandBuffer,
            this->pipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(PushConstantData),
            &push);

        object.model->bind(commandBuffer);
        object.model->draw(commandBuffer);
    }
}