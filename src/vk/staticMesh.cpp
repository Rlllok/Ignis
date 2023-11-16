#include "staticMesh.h"

#include <vector>
#include <string>
#include <iostream>
#include "tiny_obj_loader.h"

#include "vulkanUtils.hpp"

StaticMesh::StaticMesh(const Device& device, const char* path)
    : device(device)
{
    loadStaticMesh(path);
    createBuffers();
}

StaticMesh::~StaticMesh()
{
    vkFreeMemory(device.getHandle(), vertexMemory, nullptr);
    vkDestroyBuffer(device.getHandle(), vertexBuffer, nullptr);

    vkFreeMemory(device.getHandle(), indexMemory, nullptr);
    vkDestroyBuffer(device.getHandle(), indexBuffer, nullptr);
}

bool StaticMesh::loadStaticMesh(const char* path)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::string warn;
    std::string err;

    if (!tinyobj::LoadObj(&attrib, &shapes, nullptr, &warn, &err, path))
    {
        std::cerr << "Cannot load StaticMesh " << path << std::endl;
        return false;
    }

    for (const auto& shape : shapes)
    {
        for (const auto& index : shape.mesh.indices)
        {
            Vertex vertex = {};

            vertex.position.x = attrib.vertices[3 * index.vertex_index + 0];
            vertex.position.y = attrib.vertices[3 * index.vertex_index + 1];
            vertex.position.z = attrib.vertices[3 * index.vertex_index + 2];

            vertex.normal.x = attrib.normals[3 * index.normal_index + 0];
            vertex.normal.y = attrib.normals[3 * index.normal_index + 1];
            vertex.normal.z = attrib.normals[3 * index.normal_index + 2];

            vertex.textureCoordinates.x = attrib.texcoords[2 * index.texcoord_index + 0];
            vertex.textureCoordinates.y = attrib.texcoords[2 * index.texcoord_index + 1];

            vertecies.push_back(vertex);
            indecies.push_back(indecies.size());
        }
    }

    return true;
}

void StaticMesh::createBuffers()
{
    // Vertex Buffer
    VkBufferCreateInfo vertexBufferInfo = {};
    vertexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vertexBufferInfo.size = sizeof(Vertex) * vertecies.size();
    vertexBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    vertexBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device.getHandle(), &vertexBufferInfo, nullptr, &vertexBuffer) != VK_SUCCESS)
    {
        std::cerr << "Cannot create Vertex Buffer." << std::endl;
    }

    VkMemoryRequirements vertexMemRequirements = {};
    vkGetBufferMemoryRequirements(device.getHandle(), vertexBuffer, &vertexMemRequirements);

    VkMemoryPropertyFlags memProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    VkMemoryAllocateInfo vertexMemAllocateInfo = {};
    vertexMemAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vertexMemAllocateInfo.allocationSize = vertexMemRequirements.size;
    vertexMemAllocateInfo.memoryTypeIndex = vku::findMemoryType(device.getGPU(), vertexMemRequirements.memoryTypeBits, memProperties);

    if (vkAllocateMemory(device.getHandle(), &vertexMemAllocateInfo, nullptr, &vertexMemory) != VK_SUCCESS)
    {
        std::cerr << "Cannot allocate Vertex Buffer memory." << std::endl;
    }

    vkBindBufferMemory(device.getHandle(), vertexBuffer, vertexMemory, 0);

    void* data;
    vkMapMemory(device.getHandle(), vertexMemory, 0, sizeof(Vertex) * vertecies.size(), 0, &data);
    {
        memcpy(data, vertecies.data(), sizeof(Vertex) * vertecies.size());
    }
    vkUnmapMemory(device.getHandle(), vertexMemory);
 
    // Index Buffer
    VkBufferCreateInfo indexBufferInfo = {};
    indexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    indexBufferInfo.size = sizeof(uint32_t) * indecies.size();
    indexBufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    indexBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device.getHandle(), &indexBufferInfo, nullptr, &indexBuffer) != VK_SUCCESS)
    {
        std::cerr << "Cannot create Index Buffer." << std::endl;
    }

    VkMemoryRequirements indexMemRequirements = {};
    vkGetBufferMemoryRequirements(device.getHandle(), indexBuffer, &indexMemRequirements);

    VkMemoryAllocateInfo indexMemAllocateInfo = {};
    indexMemAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    indexMemAllocateInfo.allocationSize = indexMemRequirements.size;
    indexMemAllocateInfo.memoryTypeIndex = vku::findMemoryType(device.getGPU(), indexMemRequirements.memoryTypeBits, memProperties);

    if (vkAllocateMemory(device.getHandle(), &indexMemAllocateInfo, nullptr, &indexMemory) != VK_SUCCESS)
    {
        std::cerr << "Cannot create Index Buffer memory." << std::endl;
    }

    vkBindBufferMemory(device.getHandle(), indexBuffer, indexMemory, 0);

    vkMapMemory(device.getHandle(), indexMemory, 0, sizeof(uint32_t) * indecies.size(), 0, &data);
    {
        memcpy(data, indecies.data(), sizeof(uint32_t) * indecies.size());
    }
    vkUnmapMemory(device.getHandle(), indexMemory);
}
