#pragma once

#include <string>
#include <vector>

#include <Device.hpp>

struct PipelineConfigInfo
{
    VkPipelineViewportStateCreateInfo viewportInfo            = { };
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInput = { };
    VkPipelineRasterizationStateCreateInfo rasterizationInfo  = { };
    VkPipelineMultisampleStateCreateInfo multisampleInfo      = { };
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo    = { };
    std::vector<VkDynamicState> dynamicStateEnables           = { };
    VkPipelineDynamicStateCreateInfo dynamicStateInfo         = { };
    VkPipelineLayout pipelineLayout                           = nullptr;
    VkRenderPass renderPass                                   = nullptr;
    uint32_t subpass                                          = 0;
};

class Pipeline
{
private:
    Device& device;
    VkPipeline graphicsPipeline     = nullptr;
    VkShaderModule vertShaderModule = nullptr;
    VkShaderModule fragShaderModule = nullptr;

    static const std::vector<char> readFile(const std::string& filePath);
    void createGraphicsPipeline(const std::string_view& vertPath, const std::string_view& fragPath, const PipelineConfigInfo& config);
    void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

public:
    Pipeline(Device& device, const std::string_view& verPath, const std::string_view& fragPath, const PipelineConfigInfo& config);
    ~Pipeline();

    // Delete copy constructor and copy operator
    Pipeline(const Pipeline&)            = delete;
    Pipeline& operator=(const Pipeline&) = delete;

    static void defaultPipelineConfig(PipelineConfigInfo& config);
    void bind(const VkCommandBuffer& commandBuffer);
};