#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

class MyObject;
struct Vertex;

struct BBox {
    float lowX;
    float highX;
    float lowY;
    float highY;
    float lowZ;
    float highZ;
};

class MyObject { // object related stuff
public:
    bool imBBox;

    BBox bBoxAttr;

    ObjFile* objFile;

    MyObject* masterObject;
    MyObject* bBox;

    VkImage textureImage;
    VkImageView textureImageView;
    VkSampler textureSampler;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;

    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;

    std::vector<VkBuffer> m_uniformBuffers; // added
    std::vector<VkDeviceMemory> m_uniformBuffersMemory; // added

public:
    MyObject();
    MyObject(string objFilename);
    MyObject(MyObject* masterObject); // for Bounding Box only
};
