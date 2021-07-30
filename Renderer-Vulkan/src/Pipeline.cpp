#include <fstream>
#include <iostream>
#include <cassert>

#include <Pipeline.hpp>
#include <Device.hpp>
#include <Model.hpp>

Pipeline::Pipeline(Device& device, const std::string_view& vert_path, const std::string_view& frag_path, const PipelineConfigInfo& config) :
    device(device)
{
    this->createGraphicsPipeline(vert_path, frag_path, config);
}

Pipeline::~Pipeline()
{
    vkDestroyShaderModule(this->device.device(), this->vertShaderModule, nullptr);
    vkDestroyShaderModule(this->device.device(), this->fragShaderModule, nullptr);
    vkDestroyPipeline(this->device.device(), this->graphicsPipeline, nullptr);
}

const std::vector<char>
Pipeline::readFile(const std::string& file_path)
{
    std::ifstream file(file_path, std::ios::ate | std::ios::binary);

    if (!file.is_open())
        throw std::runtime_error("Failed to Open File: " + file_path);

    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}

void Pipeline::createGraphicsPipeline(const std::string_view& vertPath, const std::string_view& fragPath, const PipelineConfigInfo& config)
{
    auto vertCode = this->readFile(vertPath.data());
    auto fragCode = this->readFile(fragPath.data());

    assert(config.pipelineLayout != VK_NULL_HANDLE && "Cannot Create Graphics Pipeline, no pipeline_layout Provided in 'config'");
    assert(config.renderPass != VK_NULL_HANDLE && "Cannot Create Graphics Pipeline, no render_pass Provided in 'config'");

    this->createShaderModule(vertCode, &vertShaderModule);
    this->createShaderModule(fragCode, &fragShaderModule);

    VkPipelineShaderStageCreateInfo shaderStage[2] = { };

    shaderStage[0].sType                           = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStage[0].stage                           = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStage[0].module                          = vertShaderModule;
    shaderStage[0].pName                           = "main";
    shaderStage[0].flags                           = 0;
    shaderStage[0].pNext                           = nullptr;
    shaderStage[0].pSpecializationInfo             = nullptr;

    shaderStage[1].sType                           = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStage[1].stage                           = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStage[1].module                          = fragShaderModule;
    shaderStage[1].pName                           = "main";
    shaderStage[1].flags                           = 0;
    shaderStage[1].pNext                           = nullptr;
    shaderStage[1].pSpecializationInfo             = nullptr;

    auto bindingDescriptions                       = Model::Vertex::getBindingDescriptions();
    auto attributeDescriptions                     = Model::Vertex::getAttributeDescriptions();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo     = VkPipelineVertexInputStateCreateInfo();
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexAttributeDescriptionCount          = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.vertexBindingDescriptionCount            = static_cast<uint32_t>(bindingDescriptions.size());
    vertexInputInfo.pVertexAttributeDescriptions             = attributeDescriptions.data();
    vertexInputInfo.pVertexBindingDescriptions               = bindingDescriptions.data();

    VkPipelineColorBlendAttachmentState colorBlendAttachment = { };
    colorBlendAttachment.colorWriteMask                      =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
        VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable                         = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor                 = VK_BLEND_FACTOR_ONE;   // Optional
    colorBlendAttachment.dstColorBlendFactor                 = VK_BLEND_FACTOR_ZERO;  // Optional
    colorBlendAttachment.colorBlendOp                        = VK_BLEND_OP_ADD;       // Optional
    colorBlendAttachment.srcAlphaBlendFactor                 = VK_BLEND_FACTOR_ONE;   // Optional
    colorBlendAttachment.dstAlphaBlendFactor                 = VK_BLEND_FACTOR_ZERO;  // Optional
    colorBlendAttachment.alphaBlendOp                        = VK_BLEND_OP_ADD;       // Optional

    VkPipelineColorBlendStateCreateInfo colorBlendInfo       = { };
    colorBlendInfo.sType                                     = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendInfo.logicOpEnable                             = VK_FALSE;
    colorBlendInfo.logicOp                                   = VK_LOGIC_OP_COPY;  // Optional
    colorBlendInfo.attachmentCount                           = 1;
    colorBlendInfo.pAttachments                              = &colorBlendAttachment;
    colorBlendInfo.blendConstants[0]                         = 0.0f;              // Optional
    colorBlendInfo.blendConstants[1]                         = 0.0f;              // Optional
    colorBlendInfo.blendConstants[2]                         = 0.0f;              // Optional
    colorBlendInfo.blendConstants[3]                         = 0.0f;              // Optional

    VkGraphicsPipelineCreateInfo graphicsPipelineInfo        = VkGraphicsPipelineCreateInfo();
    graphicsPipelineInfo.sType                               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    graphicsPipelineInfo.stageCount                          = 2;
    graphicsPipelineInfo.pStages                             = shaderStage;
    graphicsPipelineInfo.pVertexInputState                   = &vertexInputInfo;
    graphicsPipelineInfo.pInputAssemblyState                 = &config.inputAssemblyInput;
    graphicsPipelineInfo.pViewportState                      = &config.viewportInfo;
    graphicsPipelineInfo.pRasterizationState                 = &config.rasterizationInfo;
    graphicsPipelineInfo.pMultisampleState                   = &config.multisampleInfo;
    graphicsPipelineInfo.pColorBlendState                    = &colorBlendInfo;
    graphicsPipelineInfo.pDepthStencilState                  = &config.depthStencilInfo;

    graphicsPipelineInfo.pDynamicState                       = &config.dynamicStateInfo;
    graphicsPipelineInfo.layout                              = config.pipelineLayout;
    graphicsPipelineInfo.renderPass                          = config.renderPass;
    graphicsPipelineInfo.subpass                             = config.subpass;

    graphicsPipelineInfo.basePipelineIndex                   = -1;
    graphicsPipelineInfo.basePipelineHandle                  = VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(this->device.device(), VK_NULL_HANDLE, 1, &graphicsPipelineInfo, nullptr, &this->graphicsPipeline) != VK_SUCCESS)
        throw std::runtime_error("Failed to Create Graphics Pipeline");
}

void Pipeline::createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule)
{
    VkShaderModuleCreateInfo createInfo = VkShaderModuleCreateInfo();
    createInfo.sType                    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize                 = code.size();
    createInfo.pCode                    = reinterpret_cast<const uint32_t*>(code.data());

    if (vkCreateShaderModule(this->device.device(), &createInfo, nullptr, shaderModule) != VK_SUCCESS)
        throw std::runtime_error("Failed to Create Shader Module");
}

void Pipeline::defaultPipelineConfig(PipelineConfigInfo& config)
{
    config.inputAssemblyInput.sType                   = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    config.inputAssemblyInput.topology                = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    config.inputAssemblyInput.primitiveRestartEnable  = VK_FALSE;

    config.viewportInfo.sType                         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    config.viewportInfo.viewportCount                 = 1;
    config.viewportInfo.pViewports                    = nullptr;
    config.viewportInfo.scissorCount                  = 1;
    config.viewportInfo.pScissors                     = nullptr;

    config.rasterizationInfo.sType                    = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    config.rasterizationInfo.depthClampEnable         = VK_FALSE;
    config.rasterizationInfo.rasterizerDiscardEnable  = VK_FALSE;
    config.rasterizationInfo.polygonMode              = VK_POLYGON_MODE_FILL;
    config.rasterizationInfo.lineWidth                = 1.f;
    config.rasterizationInfo.cullMode                 = VK_CULL_MODE_NONE;
    config.rasterizationInfo.frontFace                = VK_FRONT_FACE_CLOCKWISE;
    config.rasterizationInfo.depthBiasEnable          = VK_FALSE;
    config.rasterizationInfo.depthBiasConstantFactor  = 0.0f;  // Optional
    config.rasterizationInfo.depthBiasClamp           = 0.0f;  // Optional
    config.rasterizationInfo.depthBiasSlopeFactor     = 0.0f;  // Optional

    config.multisampleInfo.sType                      = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    config.multisampleInfo.sampleShadingEnable        = VK_FALSE;
    config.multisampleInfo.rasterizationSamples       = VK_SAMPLE_COUNT_1_BIT;
    config.multisampleInfo.minSampleShading           = 1.0f;      // Optional
    config.multisampleInfo.pSampleMask                = nullptr;   // Optional
    config.multisampleInfo.alphaToCoverageEnable      = VK_FALSE;  // Optional
    config.multisampleInfo.alphaToOneEnable           = VK_FALSE;  // Optional

    config.depthStencilInfo.sType                     = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    config.depthStencilInfo.depthTestEnable           = VK_TRUE;
    config.depthStencilInfo.depthWriteEnable          = VK_TRUE;
    config.depthStencilInfo.depthCompareOp            = VK_COMPARE_OP_LESS;
    config.depthStencilInfo.depthBoundsTestEnable     = VK_FALSE;
    config.depthStencilInfo.minDepthBounds            = 0.0f;  // Optional
    config.depthStencilInfo.maxDepthBounds            = 1.0f;  // Optional
    config.depthStencilInfo.stencilTestEnable         = VK_FALSE;
    config.depthStencilInfo.front                     = { };  // Optional
    config.depthStencilInfo.back                      = { };  // Optional

    config.dynamicStateEnables                        = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    config.dynamicStateInfo.sType                     = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    config.dynamicStateInfo.pDynamicStates            = config.dynamicStateEnables.data();
    config.dynamicStateInfo.dynamicStateCount         = static_cast<uint32_t>(config.dynamicStateEnables.size());
    config.dynamicStateInfo.flags                     = 0;
}

void Pipeline::bind(const VkCommandBuffer& command_buffer)
{
    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->graphicsPipeline);
}