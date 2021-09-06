#pragma once

#include <memory>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <Device.hpp>

class Model
{
private:
    Device& device;
    VkBuffer vertexBuffer              = nullptr;
    VkBuffer indexBuffer               = nullptr;
    VkDeviceMemory vertexBufferMemory  = nullptr;
    VkDeviceMemory indexBufferMemory   = nullptr;
    uint32_t vertexCount               = 0;
    uint32_t indexCount                = 0;
    bool hasIndexBuffer                = false;

public:
    struct Vertex
    {
        glm::vec3 position = glm::vec3();
        glm::vec3 color    = glm::vec3();
        glm::vec3 normal   = glm::vec3();
        glm::vec2 uv       = glm::vec2();

        static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

        bool operator==(const Vertex& other) const
        {
            return position == other.position &&
                   color    == other.color    &&
                   normal   == other.normal   &&
                   uv       == other.uv;
        }
    };

    struct Builder
    {
        std::vector<Vertex> vertices  = { };
        std::vector<uint32_t> indices = { };

        void loadModel(const std::string& filePath);
    };

    Model(Device& device, const Builder& builder);
    ~Model();

    // Delete copy constructor and copy operator
    Model(const Model&)            = delete;
    Model& operator=(const Model&) = delete;

    static std::unique_ptr<Model> createModelFromFile(Device& device, const std::string& filePath);

    void bind(const VkCommandBuffer& commandBuffer);
    void draw(const VkCommandBuffer& commandBuffer);

private:
    void createVertexBuffers(const std::vector<Vertex>& vertices);
    void createIndexBuffer(const std::vector<uint32_t>& indices);
};