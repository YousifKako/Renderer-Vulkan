#include <cassert>
#include <cstring>

#include <Model.hpp>

Model::Model(Device& device, const std::vector<Vertex>& vertices) :
    device(device)
{
    this->create_vertex_buffers(vertices);
}

Model::~Model()
{
    vkDestroyBuffer(this->device.device(), this->vertex_buffer, nullptr);
    vkFreeMemory(this->device.device(), this->vertex_buffer_memory, nullptr);
}

std::vector<VkVertexInputBindingDescription>
Model::Vertex::get_binding_descriptions()
{
    std::vector<VkVertexInputBindingDescription> binding_descriptions(1);
    binding_descriptions[0].binding = 0;
    binding_descriptions[0].stride = sizeof(Vertex);
    binding_descriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return binding_descriptions;
}

std::vector<VkVertexInputAttributeDescription>
Model::Vertex::get_attribute_descriptions()
{
    std::vector<VkVertexInputAttributeDescription> attribute_descriptions(2);
    attribute_descriptions[0].binding = 0;
    attribute_descriptions[0].location = 0;
    attribute_descriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
    attribute_descriptions[0].offset = offsetof(Vertex, position);

    attribute_descriptions[1].binding = 0;
    attribute_descriptions[1].location = 1;
    attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attribute_descriptions[1].offset = offsetof(Vertex, color);

    return attribute_descriptions;
}

void Model::create_vertex_buffers(const std::vector<Vertex>& vertices)
{
    this->vertex_count = static_cast<uint32_t>(vertices.size());
    assert(this->vertex_count >= 3 && "Vertex Count Must be At Least 3");

    VkDeviceSize buffer_size = sizeof(vertices[0]) * this->vertex_count;
    this->device.createBuffer(buffer_size,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        this->vertex_buffer,
        this->vertex_buffer_memory);

    void* data = nullptr;
    vkMapMemory(this->device.device(), this->vertex_buffer_memory, 0, buffer_size, 0, &data);
    memcpy(data, vertices.data(), static_cast<size_t>(buffer_size));
    vkUnmapMemory(this->device.device(), this->vertex_buffer_memory);
}

void Model::bind(const VkCommandBuffer& command_buffer)
{
    const VkBuffer buffers[] = { this->vertex_buffer };
    const VkDeviceSize offset[] = { 0 };

    vkCmdBindVertexBuffers(command_buffer, 0, 1, buffers, offset);
}

void Model::draw(const VkCommandBuffer& command_buffer)
{
    vkCmdDraw(command_buffer, this->vertex_count, 1, 0, 0);
}