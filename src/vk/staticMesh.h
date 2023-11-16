#pragma once

#include <vector>
#include <array>

#include <vulkan/vulkan.h>
// #include "tinyglTF/tiny_gltf.h"
#include <glm/glm.hpp>

#include "device.h"

class StaticMesh
{
public:
    StaticMesh(const Device& device, const char* path);
    ~StaticMesh();

    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 textureCoordinates;

        static VkVertexInputBindingDescription getBindingDescription()
        {
            VkVertexInputBindingDescription description = {};
            description.binding = 0;
            description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            description.stride = sizeof(Vertex);

            return description;
        }

        static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions()
        {
            std::array<VkVertexInputAttributeDescription, 3> attributes = {};

            attributes[0].binding = 0;
            attributes[0].location = 0;
            attributes[0].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributes[0].offset = offsetof(Vertex, position);

            attributes[1].binding = 0;
            attributes[1].location = 1;
            attributes[1].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributes[1].offset = offsetof(Vertex, normal);

            attributes[2].binding = 0;
            attributes[2].location = 2;
            attributes[2].format = VK_FORMAT_R32G32_SFLOAT;
            attributes[2].offset = offsetof(Vertex, textureCoordinates);    

            return attributes;
        }
    };

    std::vector<Vertex>     vertecies;
    std::vector<uint32_t>   indecies;

public:
    VkBuffer getVertexBuffer() const { return vertexBuffer; }
    VkBuffer getIndexBuffer() const { return indexBuffer; }
    uint32_t getIndexCount() const {return static_cast<uint32_t>(indecies.size()); }

private:
    const Device& device;

    VkBuffer vertexBuffer;
    VkDeviceMemory vertexMemory;

    VkBuffer indexBuffer;
    VkDeviceMemory indexMemory;

private:
    bool loadStaticMesh(const char* path);
    void createBuffers();
};