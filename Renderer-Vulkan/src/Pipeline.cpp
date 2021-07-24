#include <fstream>
#include <iostream>
#include <cassert>

#include <Pipeline.hpp>
#include <Device.hpp>
#include <Model.hpp>

Pipeline::Pipeline(Device& device, const std::string_view& vert_path, const std::string_view& frag_path, const PipelineConfigInfo& config) :
    device(device)
{
    this->create_graphics_pipeline(vert_path, frag_path, config);
}

Pipeline::~Pipeline()
{
    vkDestroyShaderModule(this->device.device(), this->vert_shader_module, nullptr);
    vkDestroyShaderModule(this->device.device(), this->frag_shader_module, nullptr);
    vkDestroyPipeline(this->device.device(), this->graphics_pipeline, nullptr);
}

const std::vector<char>
Pipeline::read_file(const std::string& file_path)
{
    std::ifstream file(file_path, std::ios::ate | std::ios::binary);

    if (!file.is_open())
        throw std::runtime_error("Failed to Open File: " + file_path);

    size_t file_size = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(file_size);

    file.seekg(0);
    file.read(buffer.data(), file_size);
    file.close();

    return buffer;
}

void Pipeline::create_graphics_pipeline(const std::string_view& vert_path, const std::string_view& frag_path, const PipelineConfigInfo& config)
{
    auto vert_code = this->read_file(vert_path.data());
    auto frag_code = this->read_file(frag_path.data());

    assert(config.pipeline_layout != VK_NULL_HANDLE && "Cannot Create Graphics Pipeline, no pipeline_layout Provided in 'config'");
    assert(config.render_pass != VK_NULL_HANDLE && "Cannot Create Graphics Pipeline, no render_pass Provided in 'config'");

    this->create_shader_module(vert_code, &vert_shader_module);
    this->create_shader_module(frag_code, &frag_shader_module);

    VkPipelineShaderStageCreateInfo shader_stage[2];

    shader_stage[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stage[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shader_stage[0].module = vert_shader_module;
    shader_stage[0].pName = "main";
    shader_stage[0].flags = 0;
    shader_stage[0].pNext = nullptr;
    shader_stage[0].pSpecializationInfo = nullptr;

    shader_stage[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stage[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shader_stage[1].module = frag_shader_module;
    shader_stage[1].pName = "main";
    shader_stage[1].flags = 0;
    shader_stage[1].pNext = nullptr;
    shader_stage[1].pSpecializationInfo = nullptr;

    auto binding_descriptions = Model::Vertex::get_binding_descriptions();
    auto attribute_descriptions = Model::Vertex::get_attribute_descriptions();

    VkPipelineVertexInputStateCreateInfo vertex_input_info = VkPipelineVertexInputStateCreateInfo();
    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_descriptions.size());
    vertex_input_info.vertexBindingDescriptionCount = static_cast<uint32_t>(binding_descriptions.size());
    vertex_input_info.pVertexAttributeDescriptions = attribute_descriptions.data();
    vertex_input_info.pVertexBindingDescriptions = binding_descriptions.data();

    VkPipelineColorBlendAttachmentState color_blend_attachment = { };
    color_blend_attachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
        VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment.blendEnable = VK_FALSE;
    color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
    color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
    color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;              // Optional
    color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
    color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
    color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;              // Optional

    VkPipelineColorBlendStateCreateInfo color_blend_info = { };
    color_blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend_info.logicOpEnable = VK_FALSE;
    color_blend_info.logicOp = VK_LOGIC_OP_COPY;  // Optional
    color_blend_info.attachmentCount = 1;
    color_blend_info.pAttachments = &color_blend_attachment;
    color_blend_info.blendConstants[0] = 0.0f;  // Optional
    color_blend_info.blendConstants[1] = 0.0f;  // Optional
    color_blend_info.blendConstants[2] = 0.0f;  // Optional
    color_blend_info.blendConstants[3] = 0.0f;  // Optional

    VkGraphicsPipelineCreateInfo graphics_pipeline_info = VkGraphicsPipelineCreateInfo();
    graphics_pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    graphics_pipeline_info.stageCount = 2;
    graphics_pipeline_info.pStages = shader_stage;
    graphics_pipeline_info.pVertexInputState = &vertex_input_info;
    graphics_pipeline_info.pInputAssemblyState = &config.input_assembly_input;
    graphics_pipeline_info.pViewportState = &config.viewport_info;
    graphics_pipeline_info.pRasterizationState = &config.rasterization_info;
    graphics_pipeline_info.pMultisampleState = &config.multisample_info;
    graphics_pipeline_info.pColorBlendState = &color_blend_info;
    graphics_pipeline_info.pDepthStencilState = &config.depth_stencil_info;

    graphics_pipeline_info.pDynamicState = &config.dynamic_state_info;
    graphics_pipeline_info.layout = config.pipeline_layout;
    graphics_pipeline_info.renderPass = config.render_pass;
    graphics_pipeline_info.subpass = config.subpass;

    graphics_pipeline_info.basePipelineIndex = -1;
    graphics_pipeline_info.basePipelineHandle = VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(this->device.device(), VK_NULL_HANDLE, 1, &graphics_pipeline_info, nullptr, &this->graphics_pipeline) != VK_SUCCESS)
        throw std::runtime_error("Failed to Create Graphics Pipeline");
}

void Pipeline::create_shader_module(const std::vector<char>& code, VkShaderModule* shader_module)
{
    VkShaderModuleCreateInfo create_info = VkShaderModuleCreateInfo();
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = code.size();
    create_info.pCode = reinterpret_cast<const uint32_t*>(code.data());

    if (vkCreateShaderModule(this->device.device(), &create_info, nullptr, shader_module) != VK_SUCCESS)
        throw std::runtime_error("Failed to Create Shader Module");
}

void Pipeline::default_pipeline_config(PipelineConfigInfo& config)
{
    config.input_assembly_input.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    config.input_assembly_input.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    config.input_assembly_input.primitiveRestartEnable = VK_FALSE;

    config.viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    config.viewport_info.viewportCount = 1;
    config.viewport_info.pViewports = nullptr;
    config.viewport_info.scissorCount = 1;
    config.viewport_info.pScissors = nullptr;

    config.rasterization_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    config.rasterization_info.depthClampEnable = VK_FALSE;
    config.rasterization_info.rasterizerDiscardEnable = VK_FALSE;
    config.rasterization_info.polygonMode = VK_POLYGON_MODE_FILL;
    config.rasterization_info.lineWidth = 1.f;
    config.rasterization_info.cullMode = VK_CULL_MODE_NONE;
    config.rasterization_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
    config.rasterization_info.depthBiasEnable = VK_FALSE;
    config.rasterization_info.depthBiasConstantFactor = 0.0f;  // Optional
    config.rasterization_info.depthBiasClamp = 0.0f;           // Optional
    config.rasterization_info.depthBiasSlopeFactor = 0.0f;     // Optional

    config.multisample_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    config.multisample_info.sampleShadingEnable = VK_FALSE;
    config.multisample_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    config.multisample_info.minSampleShading = 1.0f;           // Optional
    config.multisample_info.pSampleMask = nullptr;             // Optional
    config.multisample_info.alphaToCoverageEnable = VK_FALSE;  // Optional
    config.multisample_info.alphaToOneEnable = VK_FALSE;       // Optional

    config.depth_stencil_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    config.depth_stencil_info.depthTestEnable = VK_TRUE;
    config.depth_stencil_info.depthWriteEnable = VK_TRUE;
    config.depth_stencil_info.depthCompareOp = VK_COMPARE_OP_LESS;
    config.depth_stencil_info.depthBoundsTestEnable = VK_FALSE;
    config.depth_stencil_info.minDepthBounds = 0.0f;  // Optional
    config.depth_stencil_info.maxDepthBounds = 1.0f;  // Optional
    config.depth_stencil_info.stencilTestEnable = VK_FALSE;
    config.depth_stencil_info.front = {};  // Optional
    config.depth_stencil_info.back = {};   // Optional

    config.dynamic_state_enables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    config.dynamic_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    config.dynamic_state_info.pDynamicStates = config.dynamic_state_enables.data();
    config.dynamic_state_info.dynamicStateCount = static_cast<uint32_t>(config.dynamic_state_enables.size());
    config.dynamic_state_info.flags = 0;
}

void Pipeline::bind(const VkCommandBuffer& command_buffer)
{
    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->graphics_pipeline);
}