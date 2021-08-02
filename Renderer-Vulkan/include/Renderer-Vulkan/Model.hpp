#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <Device.hpp>

class Model
{
private:
    Device& device;
    VkBuffer vertexBuffer              = nullptr;
    VkDeviceMemory vertexBufferMemory  = nullptr;
    uint32_t vertexCount               = 0;

public:
    struct Vertex
    {
        glm::vec3 position = glm::vec3();
        glm::vec3 color    = glm::vec3();

        static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
    };

    Model(Device& device, const std::vector<Vertex>& vertices);
    ~Model();

    // Delete copy constructor and copy operator
    Model(const Model&)            = delete;
    Model& operator=(const Model&) = delete;

    void bind(const VkCommandBuffer& commandBuffer);
    void draw(const VkCommandBuffer& commandBuffer);

private:
    void createVertexBuffers(const std::vector<Vertex>& vertices);
};