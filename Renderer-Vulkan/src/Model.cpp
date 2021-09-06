#include <cassert>
#include <cstring>
#include <unordered_map>

#define TINYOBJLOADER_IMPLEMENTATION
#include <Objects/ObjectLoader.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <Model.hpp>
#include <Utilities.hpp>


namespace std
{
    template <>
    struct hash<Model::Vertex>
    {
        size_t operator()(Model::Vertex const& vertex) const {
            size_t seed = 0;
            hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
            return seed;
        }
    };
}

Model::Model(Device& device, const Builder& builder) :
    device(device)
{
    this->createVertexBuffers(builder.vertices);
    this->createIndexBuffer(builder.indices);
}

Model::~Model()
{
    vkDestroyBuffer(this->device.device(), this->vertexBuffer, nullptr);
    vkFreeMemory(this->device.device(), this->vertexBufferMemory, nullptr);

    if (this->hasIndexBuffer)
    {
        vkDestroyBuffer(this->device.device(), this->indexBuffer, nullptr);
        vkFreeMemory(this->device.device(), this->indexBufferMemory, nullptr);
    }
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
    attributeDescriptions[0].format   = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset   = offsetof(Vertex, position);

    attributeDescriptions[1].binding  = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format   = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset   = offsetof(Vertex, color);

    return attributeDescriptions;
}

#include <iostream>
void
Model::Builder::loadModel(const std::string& filePath)
{
    tinyobj::attrib_t attrib                   = { };
    std::vector<tinyobj::shape_t> shapes       = { };
    std::vector<tinyobj::material_t> materials = { };
    std::string warn, err                      = "";

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filePath.c_str()))
        throw std::runtime_error(warn + err);

    this->vertices.clear();
    this->indices.clear();

    std::unordered_map<Vertex, uint32_t> uniqueVertices = { };
    int objectNumber = 0;
    for (const auto& shape : shapes)
    {
        objectNumber++;
        for (const auto& index : shape.mesh.indices)
        {
            Vertex vertex = { };
            if (index.vertex_index >= 0)
            {
                vertex.position = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2],
                };
                if (objectNumber == 1)
                    vertex.color = { 1.0f, 0.0f, 0.0f };
                else if (objectNumber == 2)
                    vertex.color = { 0.0f, 1.0f, 0.0f };
                else
                    vertex.color = { 0.0f, 0.0f, 1.0f };
                //vertex.color = {
                //    attrib.colors[3 * index.vertex_index + 0],
                //    attrib.colors[3 * index.vertex_index + 1],
                //    attrib.colors[3 * index.vertex_index + 2],
                //};
            }

            if (index.normal_index >= 0)
            {
                vertex.normal = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2],
                };
            }

            if (index.texcoord_index >= 0)
            {
                vertex.uv = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    attrib.texcoords[2 * index.texcoord_index + 1],
                };
            }

            if (uniqueVertices.count(vertex) == 0)
            {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }

            indices.push_back(uniqueVertices[vertex]);
        }
    }
}

void
Model::createVertexBuffers(const std::vector<Vertex>& vertices)
{
    this->vertexCount = static_cast<uint32_t>(vertices.size());
    assert(this->vertexCount >= 3 && "Vertex Count Must be At Least 3");

    VkDeviceSize bufferSize            = sizeof(vertices[0]) * this->vertexCount;
    VkBuffer stagingBuffer             = nullptr;
    VkDeviceMemory stagingBufferMemory = nullptr;

    this->device.createBuffer(bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingBufferMemory);

    void* data = nullptr;
    vkMapMemory(this->device.device(), stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(this->device.device(), stagingBufferMemory);

    this->device.createBuffer(bufferSize,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        this->vertexBuffer,
        this->vertexBufferMemory);

    this->device.copyBuffer(stagingBuffer, this->vertexBuffer, bufferSize);

    vkDestroyBuffer(this->device.device(), stagingBuffer, nullptr);
    vkFreeMemory(this->device.device(), stagingBufferMemory, nullptr);
}

void
Model::createIndexBuffer(const std::vector<uint32_t>& indices)
{
    this->indexCount = static_cast<uint32_t>(indices.size());
    hasIndexBuffer   = indexCount > 0;

    if (!hasIndexBuffer)
        return;

    VkDeviceSize bufferSize            = sizeof(indices[0]) * this->indexCount;
    VkBuffer stagingBuffer             = nullptr;
    VkDeviceMemory stagingBufferMemory = nullptr;

    this->device.createBuffer(bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingBufferMemory);

    void* data = nullptr;
    vkMapMemory(this->device.device(), stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(this->device.device(), stagingBufferMemory);

    this->device.createBuffer(bufferSize,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        this->indexBuffer,
        this->indexBufferMemory);

    this->device.copyBuffer(stagingBuffer, this->indexBuffer, bufferSize);

    vkDestroyBuffer(this->device.device(), stagingBuffer, nullptr);
    vkFreeMemory(this->device.device(), stagingBufferMemory, nullptr);
}

std::unique_ptr<Model>
Model::createModelFromFile(Device& device, const std::string& filePath)
{
    Builder builder = { };
    builder.loadModel(filePath);

    return std::make_unique<Model>(device, builder);
}

void
Model::bind(const VkCommandBuffer& commandBuffer)
{
    const VkBuffer buffers[]    = { this->vertexBuffer };
    const VkDeviceSize offset[] = { 0 };

    vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offset);

    if (this->hasIndexBuffer)
        vkCmdBindIndexBuffer(commandBuffer, this->indexBuffer, 0, VK_INDEX_TYPE_UINT32);
}

void
Model::draw(const VkCommandBuffer& commandBuffer)
{
    if (this->hasIndexBuffer)
        vkCmdDrawIndexed(commandBuffer, this->indexCount, 1, 0, 0, 0);
    else
        vkCmdDraw(commandBuffer, this->vertexCount, 1, 0, 0);
}