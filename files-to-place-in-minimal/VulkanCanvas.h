#pragma once
#include "wx/wx.h"

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <string>
#include <vector>
#include <set>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtx/hash.hpp>

#include <iostream>
#include <fstream> 
#include <stdexcept>
#include <algorithm>
#include <chrono>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <array>
#include <optional>
#include <set>
#include <unordered_map>

using namespace std;

class ObjFile;
class GameObject;
class MyObject;
class MyMD5Object;
class MyFrame;
class MD5Model;

struct Vertex;

struct QueueFamilyIndices;

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class VulkanCanvas :
    public wxWindow
{
public:
    VulkanCanvas(wxWindow* pParent,
        wxWindowID id = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = 0,
        const wxString& name = "VulkanCanvasName",
        MyFrame* minimalMainFrame = NULL); // [1]

    virtual ~VulkanCanvas() noexcept;

private:
    void MainLoop();

    // Anything inside square brackets can be ignored, they just represent the order at which they are called
    std::vector<const char*> getRequiredExtensions();
    void InitializeVulkan(std::vector<const char*> extensions); // [2]
    VkApplicationInfo CreateApplicationInfo(const std::string& appName,
        const int32_t appVersion = VK_MAKE_VERSION(1, 0, 0),
        const std::string& engineName = "No Engine",
        const int32_t engineVersion = VK_MAKE_VERSION(1, 0, 0),
        const int32_t apiVersion = VK_API_VERSION_1_0) const noexcept; // [3]
    void CreateInstance(); // [4]
    bool CheckValidationLayerSupport(); // [4-1]
    void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo); // [4-2]
    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData); // [4-3]
    void CreateWindowSurface(); // [5]
    VkWin32SurfaceCreateInfoKHR VulkanCanvas::CreateWin32SurfaceCreateInfo() const noexcept; // [5-1]
    void PickPhysicalDevice(); // [6]
    bool IsDeviceSuitable(VkPhysicalDevice device); // [6-1]
    QueueFamilyIndices FindQueueFamilies(const VkPhysicalDevice& device) const; // [6-1-1][7-1]][12-2]
    bool CheckDeviceExtensionSupport(VkPhysicalDevice m_physicalDevice); // [6-1-2]
    SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device); // [6-1-3][8-1]
    void CreateLogicalDevice(); // [7]
    void CreateSwapChain(const wxSize& size); // [8]
    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats); // [8-2]
    VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes); // [8-3]
    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, const wxSize& size); // [8-4]
    void CreateImageViews(); // [9]
    void CreateRenderPass(); // [10]
    VkFormat FindDepthFormat(); // [10-1][14-1]
    VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features); // [10-2][14-2]
    void CreateDescriptorSetLayout(); // [11]
    void CreateGraphicsPipeline(const std::string& vertexShaderFile, const std::string& fragmentShaderFile); // [12]
    static std::vector<char> ReadFile(const std::string& filename); // [12-2][12-3]
    VkShaderModule CreateShaderModule(const std::vector<char>& code); // [12-4][12-5]
    void CreateCommandPool(); // [12]
    void CreateDepthResources(); // [13]
    void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
        VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory); // [14-2-1][16-2][17-2]
    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties); // [14-2-1-1][16-1-1][16-2-1][17-1-1][17-2-1][22-1-1][22-2-1][23-1-1][24-2-1][24-1-1][24-2-1][25-1-1][25-2-1][26-1-1][26-2-1][26-3-1]
    VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags); // [9-1][9-2][9-3][14-2-2][18-1][19-1]
    void CreateFrameBuffers(); // [15]
    void CreateTextureImage(GameObject* gameObject, string fileName, VkImage* curTextureImage); // [16][17]
    void CreateTextureImage(GameObject* gameObject, ObjFile* curObjFile, VkImage* curTextureImage); // [16][17]
    void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer,
        VkDeviceMemory& bufferMemory); // [16-1][17-1][22-1][22-2][23-1][23-2][24-1][24-2][25-1][25-2][26-1][26-2][26-3]
    void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout); // [16-3][16-5][17-3][17-5]
    VkCommandBuffer BeginSingleTimeCommands(); // [16-3-1][16-4-1][16-5-1][17-3-1][17-4-1][17-5-1][22-3-1][23-3-1][24-3-1][25-3-1]
    void EndSingleTimeCommands(VkCommandBuffer commandBuffer); // [16-3-2][16-4-2][16-5-2][17-3-2][17-4-2][17-5-2][22-3-2][23-3-2][24-3-2][25-3-2]
    void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height); // [16-4][17-4]
    void CreateTextureImageView(GameObject* gameObject, ObjFile* curObjFile, VkImage* curTextureImage, VkImageView* curTextureImageView); // [18][19]
    void CreateTextureSampler(GameObject* gameObject, VkSampler* curTextureSampler); // [20][21]

    void CreateVertexBuffer(GameObject* gameObject, ObjFile* curObjFile, MD5Model* md5Model, vector<Vertex>* curVertices, VkBuffer vertexBuffers[],
        VkDeviceMemory vertexBufferMemories[]); // [22][23]

    void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size); // [22-3][23-3][24-3][25-3]
    void CreateIndexBuffer(GameObject* gameObject, ObjFile* curObjFile, vector<uint32_t>* curIndices, VkBuffer* curIndexBuffer,
        VkDeviceMemory* curIndexBufferMemory); // [24][25]
    void CreateUniformBuffers(GameObject* gameObject, std::vector<VkBuffer>* m_uniformBuffers, std::vector<VkDeviceMemory>* m_uniformBuffersMemory); // [26]
    void CreateDescriptorPool(GameObject* gameObject, VkDescriptorPool* curDescriptorPool); // [27][28]
    void CreateDescriptorSets(GameObject* gameObject, VkDescriptorPool* curDescriptorPool, vector<VkDescriptorSet>* curDescriptorSets,
        VkImageView* curTextureImageView, VkSampler* curTextureSampler, std::vector<VkBuffer>* m_uniformBuffers); // [29][30]
    void CreateCommandBuffers(); // [31]
    void CreateSemaphores(); // [32]
    VkSemaphoreCreateInfo CreateSemaphoreCreateInfo() const noexcept; // [32-1]
    virtual void OnPaint(wxPaintEvent& event); // [33]
    VkSubmitInfo CreateSubmitInfo(uint32_t imageIndex, // [33-1]
        VkPipelineStageFlags* pipelineStageFlags) const noexcept;
    VkPresentInfoKHR CreatePresentInfoKHR(uint32_t& imageIndex) const noexcept; // [33-2]
    bool HasStencilComponent(VkFormat format);
    void UpdateUniformBuffer(GameObject* gameObject, uint32_t currentImage);

public:
    VkExtent2D m_swapchainExtent;

    // added in public for minimal.cpp to access
    float cameraX;
    float cameraY;
    float cameraZ;

    float viewingX;
    float viewingY;
    float viewingZ;

    float viewingXBeforeDrag;
    float viewingYBeforeDrag;
    float viewingZBeforeDrag;

    float cameraXBeforeDrag;
    float cameraYBeforeDrag;
    float cameraZBeforeDrag;

    float cameraXBeforeRightDrag;
    float cameraYBeforeRightDrag;
    float cameraZBeforeRightDrag;

    glm::vec3 mouseDragRay;

    glm::vec3 cameraDirection;

    glm::mat4 m_proj;
    glm::mat4 v_proj;

    glm::vec3 upVector;

    MyFrame* minimalMainFrame;

    vector<GameObject*> m_objects;

    glm::vec3 getRightVector();
    glm::vec3 getViewDir();
    void SetCameraView(glm::vec3 eye, glm::vec3 lookat, glm::vec3 up);
    void SwapAndShiftInIndexBuffer(vector<int>& tempIndexBuffer, int faceIndex);
    void DrawFrame();
    void ShowObject(string fileName, bool drawObject, bool showInSceneTree);

private:
    void UpdateVertexBuffer(MyObject* object, VkDeviceSize bufferSize);
    void SetupDebugMessenger();
    void CleanupSwapChain();
    void RecreateSwapchain();
    VkDeviceQueueCreateInfo CreateDeviceQueueCreateInfo(int queueFamily) const noexcept; // no need to be changed
    VkInstanceCreateInfo CreateInstanceCreateInfo(const VkApplicationInfo& appInfo, // no need to be changed
        const std::vector<const char*>& extensionNames,
        const std::vector<const char*>& layerNames) const noexcept;
    std::vector<VkDeviceQueueCreateInfo> VulkanCanvas::CreateQueueCreateInfos(
        const std::set<int>& uniqueQueueFamilies) const noexcept;
    VkDeviceCreateInfo CreateDeviceCreateInfo( // no need to be changed
        const std::vector<VkDeviceQueueCreateInfo>& queueCreateInfos,
        const VkPhysicalDeviceFeatures& deviceFeatures) const noexcept;
    VkAttachmentDescription CreateAttachmentDescription() const noexcept; // no need to be changed
    VkAttachmentReference CreateAttachmentReference() const noexcept; // no need to be changed
    VkSubpassDescription CreateSubpassDescription(const VkAttachmentReference& attachmentRef) const noexcept; // no need to be changed
    VkSubpassDependency CreateSubpassDependency() const noexcept; // no need to be changed

    virtual void OnResize(wxSizeEvent& event); // no need to change
    void OnPaintException(const std::string& msg); // no need to change

    wxSize windowSize;

    VkImage m_depthImage;
    VkDeviceMemory m_depthImageMemory;
    VkDeviceMemory m_textureImageMemory;
    VkDescriptorSetLayout descriptorSetLayout;
    VkImageView m_depthImageView;
    VkDebugUtilsMessengerEXT m_debugMessenger;
    std::vector<VkFence> inFlightFences;
    std::vector<VkFence> imagesInFlight;
    size_t currentFrame = 0;
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    bool framebufferResized = false;

    std::vector<MyObject*> objects;

    VkInstance m_instance;
    VkSurfaceKHR m_surface;
    VkPhysicalDevice m_physicalDevice;
    VkDevice m_logicalDevice;
    VkQueue m_graphicsQueue;
    VkQueue m_presentQueue;
    VkSwapchainKHR m_swapchain;
    std::vector<VkImage> m_swapchainImages;
    VkFormat m_swapchainImageFormat;
    //VkExtent2D m_swapchainExtent;
    std::vector<VkImageView> m_swapchainImageViews;
    VkRenderPass m_renderPass;
    VkPipelineLayout m_pipelineLayout;
    VkPipeline m_graphicsPipeline;
    std::vector<VkFramebuffer> m_swapchainFramebuffers;
    VkCommandPool m_commandPool;
    std::vector<VkCommandBuffer> m_commandBuffers;
    VkSemaphore m_imageAvailableSemaphore;
    VkSemaphore m_renderFinishedSemaphore;
    bool m_vulkanInitialized;

    int objSelected;

    int md5FrameCounter;
};

