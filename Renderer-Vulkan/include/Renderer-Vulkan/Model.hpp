#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <Device.hpp>

class Model
{
private:
    Device& device;
    VkBuffer vertex_buffer = nullptr;
    VkDeviceMemory vertex_buffer_memory = nullptr;
    uint32_t vertex_count = 0;

public:
    struct Vertex
    {
        glm::vec2 position = glm::vec2();
        glm::vec3 color = glm::vec3();

        static std::vector<VkVertexInputBindingDescription> get_binding_descriptions();
        static std::vector<VkVertexInputAttributeDescription> get_attribute_descriptions();
    };

    Model(Device& device, const std::vector<Vertex>& vertices);
    ~Model();

    // Delete copy constructor and copy operator
    Model(const Model&) = delete;
    Model& operator=(const Model&) = delete;

    void bind(const VkCommandBuffer& command_buffer);
    void draw(const VkCommandBuffer& command_buffer);

private:
    void create_vertex_buffers(const std::vector<Vertex>& vertices);
};