#pragma once

#include <string>
#include <vector>

#include <Device.hpp>

struct PipelineConfig
{
    VkViewport viewport = { };
    VkRect2D scissor = { };
    VkPipelineInputAssemblyStateCreateInfo input_assembly_input = { };
    VkPipelineRasterizationStateCreateInfo rasterization_info = { };
    VkPipelineMultisampleStateCreateInfo multisample_info = { };
    VkPipelineDepthStencilStateCreateInfo depth_stencil_info = { };
    VkPipelineLayout pipeline_layout = nullptr;
    VkRenderPass render_pass = nullptr;
    uint32_t subpass = 0;
};

class Pipeline
{
private:
    Device& device;
    VkPipeline graphics_pipeline = nullptr;
    VkShaderModule vert_shader_module = nullptr;
    VkShaderModule frag_shader_module = nullptr;

    static const std::vector<char> read_file(const std::string& file_path);
    void create_graphics_pipeline(const std::string_view& vert_path, const std::string_view& frag_path, const PipelineConfig& config);
    void create_shader_module(const std::vector<char>& code, VkShaderModule* shader_module);

public:
    Pipeline(Device& device, const std::string_view& vert_path, const std::string_view& frag_path, const PipelineConfig& config);
    ~Pipeline();

    // Delete copy constructor and copy operator
    Pipeline(const Pipeline&) = delete;
    Pipeline& operator=(const Pipeline&) = delete;

    static const PipelineConfig default_pipeline_config(const uint32_t width, uint32_t height);
    void bind(const VkCommandBuffer& command_buffer);
};