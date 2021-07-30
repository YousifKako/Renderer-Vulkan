#include <cassert>
#include <cstring>

#include <Model.hpp>

Model::Model(Device& device, const std::vector<Vertex>& vertices) :
    device(device)
{
    this->createVertexBuffers(vertices);
}

Model::~Model()
{
    vkDestroyBuffer(this->device.device(), this->vertexBuffer, nullptr);
    vkFreeMemory(this->device.device(), this->vertexBufferMemory, nullptr);
}

std::vector<VkVertexInputBindingDescription>
Model::Vertex::getBindingDescriptions()
{
    std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
    bindingDescriptions[0].binding   = 0;
    bindingDescriptions[0].stride    = sizeof(Vertex);
    bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription>
Model::Vertex::getAttributeDescriptions()
{
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);
    attributeDescriptions[0].binding  = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format   = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[0].offset   = offsetof(Vertex, position);

    attributeDescriptions[1].binding  = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format   = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset   = offsetof(Vertex, color);

    return attributeDescriptions;
}

void
Model::createVertexBuffers(const std::vector<Vertex>& vertices)
{
    this->vertexCount = static_cast<uint32_t>(vertices.size());
    assert(this->vertexCount >= 3 && "Vertex Count Must be At Least 3");

    VkDeviceSize bufferSize = sizeof(vertices[0]) * this->vertexCount;
    this->device.createBuffer(bufferSize,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        this->vertexBuffer,
        this->vertexBufferMemory);

    void* data = nullptr;
    vkMapMemory(this->device.device(), this->vertexBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(this->device.device(), this->vertexBufferMemory);
}

void
Model::bind(const VkCommandBuffer& commandBuffer)
{
    const VkBuffer buffers[]    = { this->vertexBuffer };
    const VkDeviceSize offset[] = { 0 };

    vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offset);
}

void
Model::draw(const VkCommandBuffer& commandBuffer)
{
    vkCmdDraw(commandBuffer, this->vertexCount, 1, 0, 0);
}