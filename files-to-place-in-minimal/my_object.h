#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtx/hash.hpp>

class MD5Model;
class ObjFile;
struct Vertex;

#define NUM_STAGING_BUFFER_FOR_VB 2

class GameObject { // object related stuff
public:
    bool drawObject;

    glm::vec3 offset3D;
    glm::mat4 rotationMat;

    string sceneTreeName;

    glm::mat4 curModelMat; // this shouldnt be used by md5
    std::vector<VkBuffer> m_uniformBuffers; // added
    std::vector<VkDeviceMemory> m_uniformBuffersMemory; // added

    GameObject();
    virtual ~GameObject();

    virtual bool isObjFile();
    virtual bool isMD5File();
};

class MyObject : public GameObject { // object related stuff
public:
    int startOffsetInIndexBuffer;
    int numTrianglesShownInIndexBuffer;

    bool dragObjectFlag;

    ObjFile* objFile;

    VkImage textureImage;
    VkImageView textureImageView;
    VkSampler textureSampler;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    int curVertexBufferShown;
    std::vector<VkBuffer *> stagingBufferForVB;
    std::vector<VkDeviceMemory *> stagingBufferMemoryForVB;

    std::vector<VkBuffer*> vertexBuffers;
    std::vector<VkDeviceMemory*> vertexBufferMemories;

    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;

    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;

public:
    MyObject();
    MyObject(string objFilename);
    bool isObjFile();
};

class MyMD5Object : public GameObject {
public:
    std::vector<MyObject *> myObjects; // these are basically for all the meshes

    glm::mat4 curMD5ModelMat; // do not use curModelMat inside MyObjects because the meshes will change via the seperate vertices

    int curFrame;

    MD5Model* md5Model;

    MyMD5Object();
    MyMD5Object(std::string md5Filename);

    bool isMD5File();
};
