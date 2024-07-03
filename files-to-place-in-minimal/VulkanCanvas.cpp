#define _HAS_STD_BYTE 0

#define ROTATION_SPEED 1.0f
#define MD5_FRAME_INTERVAL 5

#define CAMERA_X 10.0f
#define CAMERA_Y 10.0f
#define CAMERA_Z 10.0f

#define LOOKAT_X 0.0f
#define LOOKAT_Y 0.0f
#define LOOKAT_Z 0.0f

#include "VulkanCanvas.h"
#include "VulkanException.h"

#include <vulkan/vulkan.h>
#include <array>
#include <fstream>
#include <sstream>
#include <optional>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

using namespace std;

#include "minimal.h"
#include "scene_tree_ctrl.h"

#include "vertex.h"
#include "my_object.h"
#include "MD5Model.h"
#include "obj_file.h" // added
#include "Helpers.h"

#define AXIS_AND_GRID_FILENAME "assets/axis_and_grid/axis_and_grid.obj"

#pragma comment(lib, "vulkan-1.lib")

#define NUM_STARTING_OBJECTS 1

const int MAX_FRAMES_IN_FLIGHT = 3;

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef _DEBUG
const bool enableValidationLayers = true;
#else
const bool enableValidationLayers = false;
#endif

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);

std::vector<const char*> VulkanCanvas::getRequiredExtensions() {
    cout << "VulkanCanvas::getRequiredExtensions() <----------------------------------------------------------" << endl;
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    cout << "VulkanCanvas::getRequiredExtensions() <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;

    return extensions;
}

struct QueueFamilyIndices { // changed
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

VulkanCanvas::VulkanCanvas(wxWindow* pParent,
    wxWindowID id,
    const wxPoint& pos,
    const wxSize& size,
    long style,
    const wxString& name,
    MyFrame* minimalMainFrame)
    : wxWindow(pParent, id, pos, size, style, name),
    m_vulkanInitialized(false), m_instance(VK_NULL_HANDLE),
    m_surface(VK_NULL_HANDLE), m_physicalDevice(VK_NULL_HANDLE),
    m_logicalDevice(VK_NULL_HANDLE), m_swapchain(VK_NULL_HANDLE),
    m_renderPass(VK_NULL_HANDLE), m_pipelineLayout(VK_NULL_HANDLE),
    m_graphicsPipeline(VK_NULL_HANDLE), m_commandPool(VK_NULL_HANDLE),
    m_imageAvailableSemaphore(VK_NULL_HANDLE), m_renderFinishedSemaphore(VK_NULL_HANDLE)
{
    cout << "VulkanCanvas::VulkanCanvas(...) <----------------------------------------------------------" << endl;
    this->minimalMainFrame = minimalMainFrame;
    md5FrameCounter = MD5_FRAME_INTERVAL;

    windowSize = size;

    Bind(wxEVT_PAINT, &VulkanCanvas::OnPaint, this);
    Bind(wxEVT_SIZE, &VulkanCanvas::OnResize, this);
    std::vector<const char*> requiredExtensions = { "VK_KHR_surface", "VK_KHR_win32_surface" };
    //std::vector<const char*> requiredExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    initCamera();

    upVector = glm::vec3(0.0f, 1.0f, 0.0f);

    InitializeVulkan(requiredExtensions);
    VkApplicationInfo appInfo = CreateApplicationInfo("VulkanApp1");
    std::vector<const char*> layerNames;
    if (enableValidationLayers) {
        layerNames = validationLayers;
    }

    CreateInstance();
    CreateWindowSurface();
    PickPhysicalDevice();
    CreateLogicalDevice();
    CreateSwapChain(size);
    CreateImageViews();
    CreateRenderPass();

    CreateDescriptorSetLayout();
    //    CreateGraphicsPipeline("vert.spv", "frag.spv");
    CreateGraphicsPipeline("..\assets\vert.spv", "..\assets\frag.spv");
    CreateCommandPool();
    CreateDepthResources();
    CreateFrameBuffers();

    CreateCommandBuffers();
    CreateSemaphores();

    // add UI
    ShowObject(AXIS_AND_GRID_FILENAME, true, false);

    cout << "VulkanCanvas::VulkanCanvas(...) <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;
}

VulkanCanvas::~VulkanCanvas() noexcept
{
    cout << "VulkanCanvas::~VulkanCanvas() <----------------------------------------------------------" << endl;

    if (m_instance != VK_NULL_HANDLE) {
        if (m_logicalDevice != VK_NULL_HANDLE) {
            vkDeviceWaitIdle(m_logicalDevice);
            if (m_graphicsPipeline != VK_NULL_HANDLE) {
                vkDestroyPipeline(m_logicalDevice, m_graphicsPipeline, nullptr);
            }
            if (m_pipelineLayout != VK_NULL_HANDLE) {
                vkDestroyPipelineLayout(m_logicalDevice, m_pipelineLayout, nullptr);
            }
            if (m_renderPass != VK_NULL_HANDLE) {
                vkDestroyRenderPass(m_logicalDevice, m_renderPass, nullptr);
            }
            if (m_swapchain != VK_NULL_HANDLE) {
                vkDestroySwapchainKHR(m_logicalDevice, m_swapchain, nullptr);
            }
            for (auto& imageView : m_swapchainImageViews) {
                vkDestroyImageView(m_logicalDevice, imageView, nullptr);
            }
            for (auto& framebuffer : m_swapchainFramebuffers) {
                vkDestroyFramebuffer(m_logicalDevice, framebuffer, nullptr);
            }
            if (m_commandPool != VK_NULL_HANDLE) {
                vkDestroyCommandPool(m_logicalDevice, m_commandPool, nullptr);
            }
            if (m_imageAvailableSemaphore != VK_NULL_HANDLE) {
                vkDestroySemaphore(m_logicalDevice, m_imageAvailableSemaphore, nullptr);
            }
            if (m_renderFinishedSemaphore != VK_NULL_HANDLE) {
                vkDestroySemaphore(m_logicalDevice, m_renderFinishedSemaphore, nullptr);
            }
            vkDestroyDevice(m_logicalDevice, nullptr);
        }
        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
        vkDestroyInstance(m_instance, nullptr);
    }

    cout << "VulkanCanvas::~VulkanCanvas() <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;
}

void VulkanCanvas::InitializeVulkan(std::vector<const char*> requiredExtensions)
{
    cout << "VulkanCanvas::InitializeVulkan(...) <----------------------------------------------------------" << endl;

    // make sure that the Vulkan library is available on this system
#ifdef _WIN32
    HMODULE vulkanModule = ::LoadLibraryA("vulkan-1.dll");
    if (vulkanModule == NULL) {
        throw std::runtime_error("Vulkan library is not available on this system, so program cannot run.\n"
            "You must install the appropriate Vulkan library and also have a graphics card that supports Vulkan.");
    }
#else
#error Only Win32 is currently supported. To see how to support other windowing systems, \
 see the definition of _glfw_dlopen in XXX_platform.h and its use in vulkan.c in the glfw\
 source code. XXX specifies the windowing system (e.g. x11 for X11, and wl for Wayland).
#endif
    // make sure that the correct extensions are available
    uint32_t count;
    VkResult err = vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
    if (err != VK_SUCCESS) {
        throw VulkanException(err, "Failed to retrieve the instance extension properties:");
    }
    std::vector<VkExtensionProperties> extensions(count);
    err = vkEnumerateInstanceExtensionProperties(nullptr, &count, extensions.data());
    if (err != VK_SUCCESS) {
        throw VulkanException(err, "Failed to retrieve the instance extension properties:");
    }
    for (int extNum = 0; extNum < extensions.size(); ++extNum) {
        for (auto& iter = requiredExtensions.begin(); iter < requiredExtensions.end(); ++iter) {
            if (std::string(*iter) == extensions[extNum].extensionName) {
                requiredExtensions.erase(iter);
                break;
            }
        }
    };
    if (!requiredExtensions.empty()) {
        std::stringstream ss;
        ss << "The following required Vulkan extensions could not be found:\n";
        for (int extNum = 0; extNum < requiredExtensions.size(); ++extNum) {
            ss << requiredExtensions[extNum] << "\n";
        }
        ss << "Program cannot continue.";
        throw std::runtime_error(ss.str());
    }

    m_vulkanInitialized = true;

    cout << "VulkanCanvas::InitializeVulkan(...) <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    cout << "VulkanCanvas::CreateDebugUtilsMessengerEXT(...) <----------------------------------------------------------" << endl;
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
    cout << "VulkanCanvas::CreateDebugUtilsMessengerEXT(...) <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;
}

void VulkanCanvas::SetupDebugMessenger() {
    cout << "VulkanCanvas::SetupDebugMessenger() <----------------------------------------------------------" << endl;

    if (!enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    PopulateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &m_debugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("failed to set up debug messenger!");
    }
    cout << "VulkanCanvas::SetupDebugMessenger() <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;
}

bool VulkanCanvas::HasStencilComponent(VkFormat format) {
    cout << "VulkanCanvas::HasStencilComponent(...) <----------------------------------------------------------" << endl;
    cout << "VulkanCanvas::HasStencilComponent(...) <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;

    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void VulkanCanvas::UpdateVertexBuffer(MyObject* object, VkDeviceSize bufferSize) {
    void* data;
    vkMapMemory(m_logicalDevice, object->stagingBufferMemoryForVB[0][object->curVertexBufferShown], 0, bufferSize, 0, &data);

    memcpy(data, object->vertices.data(), (size_t)bufferSize);

    vkUnmapMemory(m_logicalDevice, object->stagingBufferMemoryForVB[0][object->curVertexBufferShown]);

    CopyBuffer(object->stagingBufferForVB[0][object->curVertexBufferShown], object->vertexBuffers[0][object->curVertexBufferShown], bufferSize);
}

void VulkanCanvas::UpdateUniformBuffer(GameObject* gameObject, uint32_t currentImage) {
    //cout << "VulkanCanvas::UpdateUniformBuffer(...) <----------------------------------------------------------" << endl;

    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
    vkResetCommandBuffer(m_commandBuffers[0], false);
    vkResetCommandBuffer(m_commandBuffers[1], false);
    vkResetCommandBuffer(m_commandBuffers[2], false);

    v_proj = glm::lookAt(glm::vec3(cameraX, cameraY, cameraZ), glm::vec3(viewingX, viewingY, viewingZ), upVector);
    m_proj = glm::perspective(glm::radians(45.0f), m_swapchainExtent.width / (float)m_swapchainExtent.height, 0.1f, 100.0f);
    for (int i = 0; i < m_objects.size(); i++) {
        UniformBufferObject ubo = {};

        if (i >= 0 && i <= NUM_STARTING_OBJECTS - 1) {
            ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0));
            ubo.model = glm::rotate(ubo.model, 0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
        }
        else {
            //glm::vec3 rot = m_objects[i]->offset3DRot;

            //glm::quat quaternionX = glm::angleAxis(glm::radians(rot.x), glm::vec3(1.0f, 0.0f, 0.0f));
            //glm::quat quaternionY = glm::angleAxis(glm::radians(rot.y), glm::vec3(0.0f, 1.0f, 0.0f));
            //glm::quat quaternionZ = glm::angleAxis(glm::radians(rot.z), glm::vec3(0.0f, 0.0f, 1.0f));

            //glm::quat combinedQuaternion = quaternionZ * quaternionY * quaternionX;
            glm::mat4 rotationMatrix = glm::toMat4(m_objects[i]->quaternion);
            ubo.model = rotationMatrix * glm::mat4(1.0f);
            ubo.model = glm::translate(glm::mat4(1.0f), m_objects[i]->offset3D) * ubo.model;

            //m_objects[i]->quaternion = combinedQuaternion;
        }

        ubo.view = v_proj;
        ubo.proj = m_proj;
        ubo.proj[1][1] *= -1;

        m_objects[i]->curModelMat = ubo.model; // keep the current model to get inversed matrix in createCommandBuffers()

        if (m_objects[i]->drawObject) {
            void* data;
            vkMapMemory(m_logicalDevice, m_objects[i]->m_uniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
            memcpy(data, &ubo, sizeof(ubo));
            vkUnmapMemory(m_logicalDevice, m_objects[i]->m_uniformBuffersMemory[currentImage]);
        }
    }
    CreateCommandBuffers();
    //cout << "VulkanCanvas::UpdateUniformBuffer(...) <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;
}

VkApplicationInfo VulkanCanvas::CreateApplicationInfo(const std::string& appName,
    const int32_t appVersion,
    const std::string& engineName,
    const int32_t engineVersion,
    const int32_t apiVersion) const noexcept
{
    cout << "VulkanCanvas::CreateApplicationInfo(...) <----------------------------------------------------------" << endl;

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Object Loader";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    cout << "VulkanCanvas::CreateApplicationInfo(...) <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;

    return appInfo;
}

VkInstanceCreateInfo VulkanCanvas::CreateInstanceCreateInfo(const VkApplicationInfo& appInfo,
    const std::vector<const char*>& extensionNames,
    const std::vector<const char*>& layerNames) const noexcept
{
    cout << "VulkanCanvas::CreateInstanceCreateInfo(...) <----------------------------------------------------------" << endl;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = extensionNames.size();
    createInfo.ppEnabledExtensionNames = extensionNames.data();
    createInfo.enabledLayerCount = layerNames.size();
    createInfo.ppEnabledLayerNames = layerNames.data();

    cout << "VulkanCanvas::CreateInstanceCreateInfo(...) <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;

    return createInfo;
}

void VulkanCanvas::CreateInstance()
{
    cout << "VulkanCanvas::CreateInstance() <----------------------------------------------------------" << endl;

    if (enableValidationLayers && !CheckValidationLayerSupport()) {
        throw std::runtime_error("validation layers requested, but not available!");
    }

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Object Loader";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo = {};
    std::vector<const char*> requiredExtensions = { "VK_KHR_surface", "VK_KHR_win32_surface" };
    std::vector<const char*> layerNames;
    if (enableValidationLayers) {
        layerNames = validationLayers;
    }

    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = requiredExtensions.size();
    createInfo.ppEnabledExtensionNames = requiredExtensions.data();
    createInfo.enabledLayerCount = layerNames.size();
    createInfo.ppEnabledLayerNames = layerNames.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        PopulateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }
    else {
        createInfo.enabledLayerCount = 0;

        createInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }

    cout << "VulkanCanvas::CreateInstance() <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;
}

#ifdef _WIN32
VkWin32SurfaceCreateInfoKHR VulkanCanvas::CreateWin32SurfaceCreateInfo() const noexcept
{
    cout << "VulkanCanvas::CreateWin32SurfaceCreateInfo() <----------------------------------------------------------" << endl;

    VkWin32SurfaceCreateInfoKHR sci = {};
    sci.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    sci.hwnd = GetHWND();
    sci.hinstance = GetModuleHandle(NULL);
    cout << "VulkanCanvas::CreateWin32SurfaceCreateInfo() <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;

    return sci;
}
#endif

void VulkanCanvas::CreateWindowSurface()
{
    cout << "VulkanCanvas::CreateWindowSurface() <----------------------------------------------------------" << endl;

    if (!m_instance) {
        throw std::runtime_error("Programming Error:\n"
            "Attempted to create a window surface before the Vulkan instance was created.");
    }
#ifdef _WIN32
    //if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
    //    throw std::runtime_error("failed to create window surface!");
    //}

    VkWin32SurfaceCreateInfoKHR sci = CreateWin32SurfaceCreateInfo();
    VkResult err = vkCreateWin32SurfaceKHR(m_instance, &sci, nullptr, &m_surface);
    //exit(1);
    if (err != VK_SUCCESS) {
        throw VulkanException(err, "Cannot create a Win32 Vulkan surface:");
    }
#else
#error The code in VulkanCanvas::CreateWindowSurface only supports Win32. Changes are \
required to support other windowing systems.
#endif
    cout << "VulkanCanvas::CreateWindowSurface() <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;

}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanCanvas::DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
    cout << "VulkanCanvas::DebugCallback(...) <----------------------------------------------------------" << endl;

    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
    cout << "VulkanCanvas::DebugCallback(...) <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;

    return VK_FALSE;
}

void VulkanCanvas::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
    cout << "VulkanCanvas::PopulateDebugMessengerCreateInfo(...) <----------------------------------------------------------" << endl;
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = DebugCallback;

    cout << "VulkanCanvas::PopulateDebugMessengerCreateInfo(...) <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;

}

bool VulkanCanvas::CheckValidationLayerSupport() {
    cout << "VulkanCanvas::CheckValidationLayerSupport() <----------------------------------------------------------" << endl;

    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }
    cout << "VulkanCanvas::CheckValidationLayerSupport() <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;

    return true;
}

void VulkanCanvas::PickPhysicalDevice()
{
    cout << "VulkanCanvas::PickPhysicalDevice() <----------------------------------------------------------" << endl;

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

    for (const auto& device : devices) {
        if (IsDeviceSuitable(device)) {
            m_physicalDevice = device;
            break;
        }
    }

    if (m_physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }

    cout << "VulkanCanvas::PickPhysicalDevice() <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;
}

bool VulkanCanvas::IsDeviceSuitable(VkPhysicalDevice device)
{
    cout << "VulkanCanvas::IsDeviceSuitable(...) <----------------------------------------------------------" << endl;

    QueueFamilyIndices indices = FindQueueFamilies(device);

    bool extensionsSupported = CheckDeviceExtensionSupport(device);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);
    cout << "VulkanCanvas::IsDeviceSuitable(...) <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;

    return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

QueueFamilyIndices VulkanCanvas::FindQueueFamilies(const VkPhysicalDevice& device) const
{
    cout << "VulkanCanvas::FindQueueFamilies(...) <----------------------------------------------------------" << endl;

    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &presentSupport);

        if (presentSupport) {
            indices.presentFamily = i;
        }

        if (indices.isComplete()) {
            break;
        }

        i++;
    }
    cout << "VulkanCanvas::FindQueueFamilies(...) <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;

    return indices;
}

bool VulkanCanvas::CheckDeviceExtensionSupport(VkPhysicalDevice device) {
    cout << "VulkanCanvas::CheckDeviceExtensionSupport(...) <----------------------------------------------------------" << endl;

    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }
    cout << "VulkanCanvas::CheckDeviceExtensionSupport(...) <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;

    return requiredExtensions.empty();
}

SwapChainSupportDetails VulkanCanvas::QuerySwapChainSupport(VkPhysicalDevice device)
{
    cout << "VulkanCanvas::QuerySwapChainSupport(...) <----------------------------------------------------------" << endl;

    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, details.presentModes.data());
    }
    cout << "VulkanCanvas::QuerySwapChainSupport(...) <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;

    return details;
}

VkDeviceQueueCreateInfo VulkanCanvas::CreateDeviceQueueCreateInfo(int queueFamily) const noexcept
{
    cout << "VulkanCanvas::CreateDeviceQueueCreateInfo(...) <----------------------------------------------------------" << endl;

    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = queueFamily;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    cout << "VulkanCanvas::CreateDeviceQueueCreateInfo(...) <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;
    return queueCreateInfo;
}

std::vector<VkDeviceQueueCreateInfo> VulkanCanvas::CreateQueueCreateInfos(
    const std::set<int>& uniqueQueueFamilies) const noexcept
{
    cout << "VulkanCanvas::CreateQueueCreateInfos(...) <----------------------------------------------------------" << endl;

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

    for (int queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo = CreateDeviceQueueCreateInfo(queueFamily);
        queueCreateInfos.push_back(queueCreateInfo);
    }
    cout << "VulkanCanvas::CreateQueueCreateInfos(...) <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;
    return queueCreateInfos;
}

VkDeviceCreateInfo VulkanCanvas::CreateDeviceCreateInfo(
    const std::vector<VkDeviceQueueCreateInfo>& queueCreateInfos,
    const VkPhysicalDeviceFeatures& deviceFeatures) const noexcept
{
    cout << "VulkanCanvas::CreateDeviceCreateInfo(...) <----------------------------------------------------------" << endl;

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = deviceExtensions.size();
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = validationLayers.size();
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else {
        createInfo.enabledLayerCount = 0;
    }
    cout << "VulkanCanvas::CreateDeviceCreateInfo(...) <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;
    return createInfo;
}

void VulkanCanvas::CreateLogicalDevice()
{
    cout << "VulkanCanvas::CreateLogicalDevice() <----------------------------------------------------------" << endl;

    QueueFamilyIndices indices = FindQueueFamilies(m_physicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_logicalDevice) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }

    vkGetDeviceQueue(m_logicalDevice, indices.graphicsFamily.value(), 0, &m_graphicsQueue);
    vkGetDeviceQueue(m_logicalDevice, indices.presentFamily.value(), 0, &m_presentQueue);
    cout << "VulkanCanvas::CreateLogicalDevice() <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;
}

void VulkanCanvas::CreateSwapChain(const wxSize& size)
{
    cout << "VulkanCanvas::CreateSwapChain(...) <----------------------------------------------------------" << endl;

    SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(m_physicalDevice);

    VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities, size);

    uint32_t imageCount = 3;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_surface;

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = FindQueueFamilies(m_physicalDevice);
    uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    if (vkCreateSwapchainKHR(m_logicalDevice, &createInfo, nullptr, &m_swapchain) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(m_logicalDevice, m_swapchain, &imageCount, nullptr);
    m_swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(m_logicalDevice, m_swapchain, &imageCount, m_swapchainImages.data());

    m_swapchainImageFormat = surfaceFormat.format;
    m_swapchainExtent = extent;
    cout << "VulkanCanvas::CreateSwapChain(...) <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;
}

VkSurfaceFormatKHR VulkanCanvas::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    cout << "VulkanCanvas::ChooseSwapSurfaceFormat(...) <----------------------------------------------------------" << endl;

    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            cout << "VulkanCanvas::ChooseSwapSurfaceFormat(...) <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;
            return availableFormat;
        }
    }
    cout << "VulkanCanvas::ChooseSwapSurfaceFormat(...) <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;

    return availableFormats[0];
}

VkPresentModeKHR VulkanCanvas::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    cout << "VulkanCanvas::ChooseSwapPresentMode(...) <----------------------------------------------------------" << endl;

    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            cout << "VulkanCanvas::ChooseSwapPresentMode(...) <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;
            return availablePresentMode;
        }
    }
    cout << "VulkanCanvas::ChooseSwapPresentMode(...) <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanCanvas::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, const wxSize& size) {
    cout << "VulkanCanvas::ChooseSwapExtent(...) <----------------------------------------------------------" << endl;

    if (capabilities.currentExtent.width != UINT32_MAX) {
        cout << "VulkanCanvas::ChooseSwapExtent(...) <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;
        return capabilities.currentExtent;
    }
    else {
        int width, height;
        //glfwGetFramebufferSize(window, &width, &height);
        width = size.x;
        height = size.y;

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

        cout << "VulkanCanvas::ChooseSwapExtent(...) <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;
        return actualExtent;
    }
    cout << "VulkanCanvas::ChooseSwapExtent(...) <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;
}

void VulkanCanvas::CreateImageViews()
{
    cout << "VulkanCanvas::CreateImageViews() <----------------------------------------------------------" << endl;

    m_swapchainImageViews.resize(m_swapchainImages.size());

    for (uint32_t i = 0; i < m_swapchainImages.size(); i++) {
        m_swapchainImageViews[i] = CreateImageView(m_swapchainImages[i], m_swapchainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
    }
    /*m_swapchainImageViews.resize(m_swapchainImages.size());
    for (uint32_t i = 0; i < m_swapchainImages.size(); i++) {
        VkImageViewCreateInfo createInfo = CreateImageViewCreateInfo(i);

        VkResult result = vkCreateImageView(m_logicalDevice, &createInfo, nullptr, &m_swapchainImageViews[i]);
        if (result != VK_SUCCESS) {
            throw VulkanException(result, "Unable to create an image view for a swap chain image");
        }
    }*/
    cout << "VulkanCanvas::CreateImageViews() <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;
}
VkImageView VulkanCanvas::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
    //cout << "VulkanCanvas::CreateImageView(...) <----------------------------------------------------------" << endl;

    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (vkCreateImageView(m_logicalDevice, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }
    //cout << "VulkanCanvas::CreateImageView(...) <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;

    return imageView;
}
VkAttachmentDescription VulkanCanvas::CreateAttachmentDescription() const noexcept
{
    cout << "VulkanCanvas::CreateAttachmentDescription() <----------------------------------------------------------" << endl;

    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = m_swapchainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    cout << "VulkanCanvas::CreateAttachmentDescription() <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;
    return colorAttachment;
}

VkAttachmentReference VulkanCanvas::CreateAttachmentReference() const noexcept
{
    cout << "VulkanCanvas::CreateAttachmentReference() <----------------------------------------------------------" << endl;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    cout << "VulkanCanvas::CreateAttachmentReference() <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;
    return colorAttachmentRef;
}

VkSubpassDescription VulkanCanvas::CreateSubpassDescription(
    const VkAttachmentReference& attachmentRef) const noexcept
{
    VkSubpassDescription subPass = {};
    subPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subPass.colorAttachmentCount = 1;
    subPass.pColorAttachments = &attachmentRef;
    cout << "VulkanCanvas::CreateAttachmentReference(...) <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;
    return subPass;
}

VkSubpassDependency VulkanCanvas::CreateSubpassDependency() const noexcept
{
    cout << "VulkanCanvas::CreateSubpassDependency() <----------------------------------------------------------" << endl;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependency.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    cout << "VulkanCanvas::CreateSubpassDependency() <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;
    return dependency;
}

void VulkanCanvas::CreateRenderPass()
{
    cout << "VulkanCanvas::CreateRenderPass() <----------------------------------------------------------" << endl;

    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = m_swapchainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentDescription depthAttachment = {};
    depthAttachment.format = FindDepthFormat();
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef = {};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(m_logicalDevice, &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }

    cout << "VulkanCanvas::CreateRenderPass() <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;
}

VkFormat VulkanCanvas::FindDepthFormat() {
    cout << "VulkanCanvas::FindDepthFormat() <----------------------------------------------------------" << endl;
    cout << "VulkanCanvas::FindDepthFormat() <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;
    return FindSupportedFormat(
        { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

void VulkanCanvas::CreateTextureImage(GameObject* gameObject, ObjFile* curObjFile, VkImage* curTextureImage) {
    //cout << "VulkanCanvas::CreateTextureImage(...) <----------------------------------------------------------" << endl;
    int texWidth, texHeight, texChannels;
    //cout << " objFile->materials.size(): " << curObjFile->materials.size() << endl;
    string fullPathOfTexture = curObjFile->pathOnly + curObjFile->materials[0]->fileNameOfTextureMap;
    //cout << "curObjFile->pathOnly + curObjFile->materials[0]->fileNameOfTextureMap: " << fullPathOfTexture << endl;
    stbi_uc* pixels = stbi_load(fullPathOfTexture.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(m_logicalDevice, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(m_logicalDevice, stagingBufferMemory);

    stbi_image_free(pixels);

    CreateImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, *curTextureImage, m_textureImageMemory);

    TransitionImageLayout(*curTextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    CopyBufferToImage(stagingBuffer, *curTextureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
    TransitionImageLayout(*curTextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(m_logicalDevice, stagingBuffer, nullptr);
    vkFreeMemory(m_logicalDevice, stagingBufferMemory, nullptr);
    //cout << "VulkanCanvas::CreateTextureImage(...) <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;
}

void VulkanCanvas::CreateTextureImage(GameObject* gameObject, string fileName, VkImage* curTextureImage) {
    //cout << "VulkanCanvas::CreateTextureImage(...) <----------------------------------------------------------" << endl;
    MyMD5Object* myMD5Object = (MyMD5Object*)gameObject;
    for (int i = 0; i < myMD5Object->md5Model->m_Meshes.size(); i++) {
        VkImage* curTextureImage = &myMD5Object->myObjects[i]->textureImage;
        int texWidth, texHeight, texChannels;

        cout << fileName << endl;
        cout << myMD5Object->md5Model->m_Meshes[i].m_Shader << endl;
        string fullPathOfTexture = "assets\\md5_anim\\" + fileName + "\\" + myMD5Object->md5Model->m_Meshes[i].m_Shader + ".png";
        //cout << "curObjFile->pathOnly + curObjFile->materials[0]->fileNameOfTextureMap: " << fullPathOfTexture << endl;
        stbi_uc* pixels = stbi_load(fullPathOfTexture.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

        VkDeviceSize imageSize = texWidth * texHeight * 4;

        if (!pixels) {
            throw std::runtime_error("failed to load texture image!");
        }

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(m_logicalDevice, stagingBufferMemory, 0, imageSize, 0, &data);
        memcpy(data, pixels, static_cast<size_t>(imageSize));
        vkUnmapMemory(m_logicalDevice, stagingBufferMemory);

        stbi_image_free(pixels);

        CreateImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, *curTextureImage, m_textureImageMemory);

        TransitionImageLayout(*curTextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        CopyBufferToImage(stagingBuffer, *curTextureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
        TransitionImageLayout(*curTextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        vkDestroyBuffer(m_logicalDevice, stagingBuffer, nullptr);
        vkFreeMemory(m_logicalDevice, stagingBufferMemory, nullptr);
    }
}

void VulkanCanvas::CreateTextureImageView(GameObject* gameObject, ObjFile* curObjFile, VkImage* curTextureImage, VkImageView* curTextureImageView) {
    //cout << "VulkanCanvas::CreateTextureImageView(...) <----------------------------------------------------------" << endl;
    if (gameObject->isObjFile()) {
        *curTextureImageView = CreateImageView(*curTextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
    }
    else {
        MyMD5Object* myMD5Object = (MyMD5Object*)gameObject;
        VkImageView* curTextureImageViewMD5;
        VkImage* curTextureImageMD5;
        for (int i = 0; i < myMD5Object->md5Model->m_Meshes.size(); i++) {
            curTextureImageViewMD5 = &myMD5Object->myObjects[i]->textureImageView;
            curTextureImageMD5 = &myMD5Object->myObjects[i]->textureImage;
            *curTextureImageViewMD5 = CreateImageView(*curTextureImageMD5, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
        }
    }
    //cout << "VulkanCanvas::CreateTextureImageView(...) <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;
}

void VulkanCanvas::CreateTextureSampler(GameObject* gameObject, VkSampler* curTextureSampler) {
    cout << "VulkanCanvas::CreateTextureSampler(...) <----------------------------------------------------------" << endl;

    if (gameObject->isObjFile()) {
        VkSamplerCreateInfo samplerInfo = {};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = 16;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

        if (vkCreateSampler(m_logicalDevice, &samplerInfo, nullptr, curTextureSampler) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture sampler!");
        }
    }
    else {
        MyMD5Object* myMD5Object = (MyMD5Object*)gameObject;
        VkSampler* curTextureSamplerMD5;
        for (int i = 0; i < myMD5Object->md5Model->m_Meshes.size(); i++) {
            curTextureSamplerMD5 = &myMD5Object->myObjects[i]->textureSampler;

            VkSamplerCreateInfo samplerInfo = {};
            samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            samplerInfo.magFilter = VK_FILTER_LINEAR;
            samplerInfo.minFilter = VK_FILTER_LINEAR;
            samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            samplerInfo.anisotropyEnable = VK_TRUE;
            samplerInfo.maxAnisotropy = 16;
            samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
            samplerInfo.unnormalizedCoordinates = VK_FALSE;
            samplerInfo.compareEnable = VK_FALSE;
            samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
            samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

            if (vkCreateSampler(m_logicalDevice, &samplerInfo, nullptr, curTextureSamplerMD5) != VK_SUCCESS) {
                throw std::runtime_error("failed to create texture sampler!");
            }
        }
    }
    cout << "VulkanCanvas::CreateTextureSampler(...) <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;
}

void VulkanCanvas::CreateVertexBuffer(GameObject* gameObject, ObjFile* curObjFile, MD5Model* md5Model, vector<Vertex>* curVertices, VkBuffer vertexBuffers[], VkDeviceMemory vertexBufferMemories[]) {
    cout << "VulkanCanvas::CreateVertexBuffer(...) <----------------------------------------------------------" << endl;
    // .obj?
    if (gameObject->isObjFile()) {
        MyObject* myObject = (MyObject*)gameObject;
        VkDeviceSize bufferSize = sizeof(Vertex) * curObjFile->vertexBuffer.size();
        for (int i = 0; i < curObjFile->vertexBuffer.size(); i++) {
            Vertex curVertex;
            curVertex.pos.x = curObjFile->vertexBuffer[i].position.x;
            curVertex.pos.y = curObjFile->vertexBuffer[i].position.y;
            curVertex.pos.z = curObjFile->vertexBuffer[i].position.z;
            curVertex.color.r = 0;
            curVertex.color.g = 0;
            curVertex.color.b = 0;
            curVertex.texCoord.s = curObjFile->vertexBuffer[i].texCoord.s;
            curVertex.texCoord.t = curObjFile->vertexBuffer[i].texCoord.t;
            curVertices->push_back(curVertex);
        }

        myObject->curVertexBufferShown = 0;

        for (int i = 0; i < NUM_STAGING_BUFFER_FOR_VB; i++) {
            CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, myObject->stagingBufferForVB[0][i], myObject->stagingBufferMemoryForVB[0][i]);

            void* data;
            vkMapMemory(m_logicalDevice, myObject->stagingBufferMemoryForVB[0][i], 0, bufferSize, 0, &data);
            memcpy(data, curVertices->data(), (size_t)bufferSize);

            vkUnmapMemory(m_logicalDevice, myObject->stagingBufferMemoryForVB[0][i]);

            CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, myObject->vertexBuffers[0][i], myObject->vertexBufferMemories[0][i]);

            CopyBuffer(myObject->stagingBufferForVB[0][i], myObject->vertexBuffers[0][i], bufferSize);

            // do not destroy
            //vkDestroyBuffer(m_logicalDevice, stagingBuffer, nullptr);
            //vkFreeMemory(m_logicalDevice, stagingBufferMemory, nullptr);
        }
    }
    // md5?
    else {
        MyMD5Object* myMD5Object = (MyMD5Object*)gameObject;

        //vector<MD5Model::JointList> jointLists;

        for (int i = 0; i < myMD5Object->md5Model->m_Animation.m_iNumFrames; i++) {
            //cout << "-------------------------------------------------------------------" << endl;
            //cout << "Frame: " << i << endl;
            //cout << "-------------------------------------------------------------------" << endl;

            MD5Model::JointList jointlist; // all joints for one frame

            for (int j = 0; j < myMD5Object->md5Model->m_Joints.size(); j++) {
                unsigned int k = 0;
                MD5Model::Joint curJoint;

                //curJoint.m_Pos = myMD5Object->md5Model->m_Animation.m_BaseFrames[j].m_Pos;
                //curJoint.m_Orient = myMD5Object->md5Model->m_Animation.m_BaseFrames[j].m_Orient;
                curJoint.m_ParentID = myMD5Object->md5Model->m_Animation.m_JointInfos[j].m_ParentID;

                const MD5Animation::JointInfo& jointInfo = myMD5Object->md5Model->m_Animation.m_JointInfos[j];
                const MD5Animation::FrameData& frameData = myMD5Object->md5Model->m_Animation.m_Frames[i];
                if (jointInfo.m_Flags & 1) // Pos.x
                {
                    curJoint.m_Pos.x = frameData.m_FrameData[jointInfo.m_StartIndex + k++];
                }
                if (jointInfo.m_Flags & 2) // Pos.y
                {
                    curJoint.m_Pos.y = frameData.m_FrameData[jointInfo.m_StartIndex + k++];
                }
                if (jointInfo.m_Flags & 4) // Pos.x
                {
                    curJoint.m_Pos.z = frameData.m_FrameData[jointInfo.m_StartIndex + k++];
                }
                if (jointInfo.m_Flags & 8) // Orient.x
                {
                    curJoint.m_Orient.x = frameData.m_FrameData[jointInfo.m_StartIndex + k++];
                }
                if (jointInfo.m_Flags & 16) // Orient.y
                {
                    curJoint.m_Orient.y = frameData.m_FrameData[jointInfo.m_StartIndex + k++];
                }
                if (jointInfo.m_Flags & 32) // Orient.z
                {
                    curJoint.m_Orient.z = frameData.m_FrameData[jointInfo.m_StartIndex + k++];
                }

                ComputeQuatW(curJoint.m_Orient);

                if (curJoint.m_ParentID >= 0) // Has a parent joint glm::translate(curJoint.m_JointMatrix, glm::vec3(curJoint.m_Pos.x - parentJoint.m_Pos.x, curJoint.m_Pos.y - parentJoint.m_Pos.y, curJoint.m_Pos.z - parentJoint.m_Pos.z));
                {
                    MD5Model::Joint& parentJoint = jointlist[curJoint.m_ParentID];
                    glm::vec3 rotPos = parentJoint.m_Orient * curJoint.m_Pos;

                    curJoint.m_Pos = parentJoint.m_Pos + rotPos;
                    curJoint.m_Orient = parentJoint.m_Orient * curJoint.m_Orient;

                    // To create joint final matrix
                    //curJoint.m_JointMatrix = glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(curJoint.m_Orient.x / parentJoint.m_Orient.x, curJoint.m_Orient.y / parentJoint.m_Orient.y, curJoint.m_Orient.z / parentJoint.m_Orient.z));
                    //curJoint.m_JointMatrix *=
                    ////    curJoint.m_JointMatrix *= parentJoint;
                    //}

                    curJoint.m_Orient = glm::normalize(curJoint.m_Orient);

                }
                jointlist.push_back(curJoint);
                //cout << "Joint " << myMD5Object->md5Model->m_Joints[j].m_Name << ": " << curJoint.m_Pos.x << ", " << curJoint.m_Pos.y << ", " << curJoint.m_Pos.z << endl;

            }
            // calculate final positions and orientations for every vertex for current frame by using joint position and orientation
            for (int j = 0; j < myMD5Object->myObjects.size(); j++) {

                vector<Vertex> verticesForCurFrame;

                for (unsigned int k = 0; k < myMD5Object->md5Model->m_Meshes[j].m_Verts.size(); k++)
                {
                    Vertex curVertex;

                    curVertex.texCoord.s = myMD5Object->md5Model->m_Meshes[j].m_Tex2DBuffer[k].s;
                    curVertex.texCoord.t = myMD5Object->md5Model->m_Meshes[j].m_Tex2DBuffer[k].t;

                    const MD5Model::Vertex& vert = myMD5Object->md5Model->m_Meshes[j].m_Verts[k];
                    glm::vec3 pos = myMD5Object->md5Model->m_Meshes[j].m_PositionBuffer[k];
                    glm::vec3 normal = myMD5Object->md5Model->m_Meshes[j].m_NormalBuffer[k];

                    pos = glm::vec3(0);
                    normal = glm::vec3(0);

                    for (int l = 0; l < vert.m_WeightCount; l++)
                    {
                        const MD5Model::Weight& weight = myMD5Object->md5Model->m_Meshes[j].m_Weights[vert.m_StartWeight + l];
                        const MD5Model::Joint& joint = jointlist[weight.m_JointID];

                        glm::vec3 rotPos = joint.m_Orient * weight.m_Pos;
                        pos += (joint.m_Pos + rotPos) * weight.m_Bias;

                        normal += (joint.m_Orient * vert.m_Normal) * weight.m_Bias;
                    }
                    curVertex.pos.x = pos.x;
                    curVertex.pos.y = pos.y;
                    curVertex.pos.z = pos.z;
                    verticesForCurFrame.push_back(curVertex);
                }


                VkDeviceSize bufferSize = sizeof(Vertex) * myMD5Object->myObjects[j]->vertices.size();
                myMD5Object->myObjects[j]->curVertexBufferShown = 0;

                for (int k = 0; k < NUM_STAGING_BUFFER_FOR_VB; k++) {
                    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, myMD5Object->myObjects[j]->stagingBufferForVB[i][k], myMD5Object->myObjects[j]->stagingBufferMemoryForVB[i][k]);

                    void* data;
                    vkMapMemory(m_logicalDevice, myMD5Object->myObjects[j]->stagingBufferMemoryForVB[i][k], 0, bufferSize, 0, &data);
                    //memcpy(data, vertices.data(), (size_t)bufferSize);
                    vector<Vertex>* verticesMD5 = &verticesForCurFrame;
                    memcpy(data, verticesMD5->data(), (size_t)bufferSize);

                    vkUnmapMemory(m_logicalDevice, myMD5Object->myObjects[j]->stagingBufferMemoryForVB[i][k]);

                    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, myMD5Object->myObjects[j]->vertexBuffers[i][k], myMD5Object->myObjects[j]->vertexBufferMemories[i][k]);

                    CopyBuffer(myMD5Object->myObjects[j]->stagingBufferForVB[i][k], myMD5Object->myObjects[j]->vertexBuffers[i][k], bufferSize);

                    // do not destroy
                    //vkDestroyBuffer(m_logicalDevice, stagingBuffer, nullptr);
                    //vkFreeMemory(m_logicalDevice, stagingBufferMemory, nullptr);
                }
            }
        }
    }
    cout << "VulkanCanvas::CreateVertexBuffer(...) <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;
}

// the function gets a copy of an index buffer and an offset which represents which face to shift
// the facs goes in a temp variable and everything gets shifted
void VulkanCanvas::SwapAndShiftInIndexBuffer(vector<int>& tempIndexBuffer, int faceIndex) {
    if (faceIndex == 11) {
        return;
    }

    int temp1 = tempIndexBuffer[faceIndex * 3];
    int temp2 = tempIndexBuffer[faceIndex * 3 + 1];
    int temp3 = tempIndexBuffer[faceIndex * 3 + 2];

    for (int i = 0; i < tempIndexBuffer.size(); i++) {
        cout << "SwapAndShiftInIndexBuffer(): Before: tempIndexBuffer[i]: " << tempIndexBuffer[i] << endl;
    }
    cout << "SwapAndShiftInIndexBuffer(): faceIndex: " << faceIndex << endl;
    // shift
    for (int i = (faceIndex + 1) * 3; i < tempIndexBuffer.size(); i++) {
        tempIndexBuffer[i - 3] = tempIndexBuffer[i];
    }

    tempIndexBuffer[tempIndexBuffer.size() - 3] = temp1;
    tempIndexBuffer[tempIndexBuffer.size() - 2] = temp2;
    tempIndexBuffer[tempIndexBuffer.size() - 1] = temp3;
    for (int i = 0; i < tempIndexBuffer.size(); i++) {
        cout << "SwapAndShiftInIndexBuffer(): After: tempIndexBuffer[i]: " << tempIndexBuffer[i] << endl;
    }
}
void VulkanCanvas::CreateIndexBuffer(GameObject* gameObject, ObjFile* curObjFile, vector<uint32_t>* curIndices, VkBuffer* curIndexBuffer, VkDeviceMemory* curIndexBufferMemory) {
    cout << "VulkanCanvas::CreateIndexBuffer(...) <----------------------------------------------------------" << endl;

    if (gameObject->isObjFile()) {
        VkDeviceSize bufferSize = sizeof(uint32_t) * curObjFile->indexBuffer.size();
        for (int i = 0; i < curObjFile->indexBuffer.size(); i++) {
            uint32_t curIndex = curObjFile->indexBuffer[i]; // ??????????????????????????????????????????????  MUST not use the number greater than 2,147,483,647, uint32_t goes up to 2,147,483,647 * 2 - 1
            curIndices->push_back(curIndex);
        }

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(m_logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
        //memcpy(data, indices.data(), (size_t)bufferSize);
        memcpy(data, curIndices->data(), (size_t)bufferSize);
        vkUnmapMemory(m_logicalDevice, stagingBufferMemory);

        CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, *curIndexBuffer, *curIndexBufferMemory);

        CopyBuffer(stagingBuffer, *curIndexBuffer, bufferSize);

        vkDestroyBuffer(m_logicalDevice, stagingBuffer, nullptr);
        vkFreeMemory(m_logicalDevice, stagingBufferMemory, nullptr);
    }
    else {
        MyMD5Object* myMD5Object = (MyMD5Object*)gameObject;
        vector<uint32_t>* curIndicesMD5;
        VkBuffer* curIndexBufferMD5;
        VkDeviceMemory* curIndexBufferMemoryMD5;
        for (int i = 0; i < myMD5Object->myObjects.size(); i++) {

            VkDeviceSize bufferSize = sizeof(uint32_t) * myMD5Object->myObjects[i]->indices.size();

            curIndicesMD5 = &myMD5Object->myObjects[i]->indices;
            curIndexBufferMD5 = &myMD5Object->myObjects[i]->indexBuffer;
            curIndexBufferMemoryMD5 = &myMD5Object->myObjects[i]->indexBufferMemory;

            VkBuffer stagingBuffer;
            VkDeviceMemory stagingBufferMemory;
            CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

            void* data;
            vkMapMemory(m_logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
            memcpy(data, curIndicesMD5->data(), (size_t)bufferSize);
            vkUnmapMemory(m_logicalDevice, stagingBufferMemory);

            CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, *curIndexBufferMD5, *curIndexBufferMemoryMD5);

            CopyBuffer(stagingBuffer, *curIndexBufferMD5, bufferSize);

            vkDestroyBuffer(m_logicalDevice, stagingBuffer, nullptr);
            vkFreeMemory(m_logicalDevice, stagingBufferMemory, nullptr);
        }
    }
    cout << "VulkanCanvas::CreateIndexBuffer(...) <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;
}

void VulkanCanvas::CreateUniformBuffers(GameObject* gameObject, std::vector<VkBuffer>* m_uniformBuffers, std::vector<VkDeviceMemory>* m_uniformBuffersMemory) {
    cout << "VulkanCanvas::CreateUniformBuffers() <----------------------------------------------------------" << endl;

    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    m_uniformBuffers->resize(m_swapchainImages.size());
    m_uniformBuffersMemory->resize(m_swapchainImages.size());

    for (size_t j = 0; j < m_swapchainImages.size(); j++) {
        CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_uniformBuffers->at(j), m_uniformBuffersMemory->at(j));
    }

    cout << "VulkanCanvas::CreateUniformBuffers() <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;
}

void VulkanCanvas::CreateDescriptorPool(GameObject* gameObject, VkDescriptorPool* curDescriptorPool) {
    cout << "VulkanCanvas::CreateDescriptorPool(...) <----------------------------------------------------------" << endl;

    if (gameObject->isObjFile()) {
        std::array<VkDescriptorPoolSize, 2> poolSizes = {};
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = static_cast<uint32_t>(m_swapchainImages.size());
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = static_cast<uint32_t>(m_swapchainImages.size());

        VkDescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = static_cast<uint32_t>(m_swapchainImages.size());

        if (vkCreateDescriptorPool(m_logicalDevice, &poolInfo, nullptr, curDescriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }
    else {
        MyMD5Object* myMD5Object = (MyMD5Object*)gameObject;
        VkDescriptorPool* curDescriptorPoolMD5;
        for (int i = 0; i < myMD5Object->myObjects.size(); i++) {
            curDescriptorPoolMD5 = &myMD5Object->myObjects[i]->descriptorPool;
            std::array<VkDescriptorPoolSize, 2> poolSizes = {};
            poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            poolSizes[0].descriptorCount = static_cast<uint32_t>(m_swapchainImages.size());
            poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            poolSizes[1].descriptorCount = static_cast<uint32_t>(m_swapchainImages.size());

            VkDescriptorPoolCreateInfo poolInfo = {};
            poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
            poolInfo.pPoolSizes = poolSizes.data();
            poolInfo.maxSets = static_cast<uint32_t>(m_swapchainImages.size());

            if (vkCreateDescriptorPool(m_logicalDevice, &poolInfo, nullptr, curDescriptorPoolMD5) != VK_SUCCESS) {
                throw std::runtime_error("failed to create descriptor pool!");
            }
        }
    }
    cout << "VulkanCanvas::CreateDescriptorPool(...) <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;
}

void VulkanCanvas::CreateDescriptorSets(GameObject* gameObject, VkDescriptorPool* curDescriptorPool, vector<VkDescriptorSet>* curDescriptorSets, VkImageView* curTextureImageView, VkSampler* curTextureSampler, std::vector<VkBuffer>* m_uniformBuffers) {
    cout << "VulkanCanvas::CreateDescriptorSets(...) <----------------------------------------------------------" << endl;

    if (gameObject->isObjFile()) {
        std::vector<VkDescriptorSetLayout> layouts(m_swapchainImages.size(), descriptorSetLayout);
        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = *curDescriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(m_swapchainImages.size());
        allocInfo.pSetLayouts = layouts.data();

        curDescriptorSets->resize(m_swapchainImages.size());
        if (vkAllocateDescriptorSets(m_logicalDevice, &allocInfo, curDescriptorSets->data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }

        for (size_t i = 0; i < m_swapchainImages.size(); i++) {
            VkDescriptorBufferInfo bufferInfo = {};
            bufferInfo.buffer = m_uniformBuffers->at(i);
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);

            VkDescriptorImageInfo imageInfo = {};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = *curTextureImageView;
            imageInfo.sampler = *curTextureSampler;

            std::array<VkWriteDescriptorSet, 2> descriptorWrites = {};

            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = (*curDescriptorSets)[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;

            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = (*curDescriptorSets)[i];
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pImageInfo = &imageInfo;

            vkUpdateDescriptorSets(m_logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }
    }
    else {
        MyMD5Object* myMD5Object = (MyMD5Object*)gameObject;
        VkDescriptorPool* curDescriptorPoolMD5;
        vector<VkDescriptorSet>* curDescriptorSetsMD5;
        VkImageView* curTextureImageViewMD5;
        VkSampler* curTextureSamplerMD5;
        std::vector<VkBuffer>* m_uniformBuffersMD5;
        for (int i = 0; i < myMD5Object->myObjects.size(); i++) {
            curDescriptorPoolMD5 = &myMD5Object->myObjects[i]->descriptorPool;
            curDescriptorSetsMD5 = &myMD5Object->myObjects[i]->descriptorSets;
            curTextureImageViewMD5 = &myMD5Object->myObjects[i]->textureImageView;
            curTextureSamplerMD5 = &myMD5Object->myObjects[i]->textureSampler;
            m_uniformBuffersMD5 = &myMD5Object->m_uniformBuffers; // we use only one main uniformbuffer
            std::vector<VkDescriptorSetLayout> layouts(m_swapchainImages.size(), descriptorSetLayout);
            VkDescriptorSetAllocateInfo allocInfo = {};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = *curDescriptorPoolMD5;
            allocInfo.descriptorSetCount = static_cast<uint32_t>(m_swapchainImages.size());
            allocInfo.pSetLayouts = layouts.data();

            curDescriptorSetsMD5->resize(m_swapchainImages.size());
            if (vkAllocateDescriptorSets(m_logicalDevice, &allocInfo, curDescriptorSetsMD5->data()) != VK_SUCCESS) {
                throw std::runtime_error("failed to allocate descriptor sets!");
            }

            for (size_t j = 0; j < m_swapchainImages.size(); j++) {
                VkDescriptorBufferInfo bufferInfo = {};
                bufferInfo.buffer = m_uniformBuffersMD5->at(j);
                bufferInfo.offset = 0;
                bufferInfo.range = sizeof(UniformBufferObject);

                VkDescriptorImageInfo imageInfo = {};
                imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                imageInfo.imageView = *curTextureImageViewMD5;
                imageInfo.sampler = *curTextureSamplerMD5;

                std::array<VkWriteDescriptorSet, 2> descriptorWrites = {};

                descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrites[0].dstSet = (*curDescriptorSetsMD5)[j];
                descriptorWrites[0].dstBinding = 0;
                descriptorWrites[0].dstArrayElement = 0;
                descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                descriptorWrites[0].descriptorCount = 1;
                descriptorWrites[0].pBufferInfo = &bufferInfo;

                descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrites[1].dstSet = (*curDescriptorSetsMD5)[j];
                descriptorWrites[1].dstBinding = 1;
                descriptorWrites[1].dstArrayElement = 0;
                descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                descriptorWrites[1].descriptorCount = 1;
                descriptorWrites[1].pImageInfo = &imageInfo;

                vkUpdateDescriptorSets(m_logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
            }
        }
    }

    cout << "VulkanCanvas::CreateDescriptorSets(...) <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;
}

void VulkanCanvas::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
    //cout << "VulkanCanvas::CopyBufferToImage(...) <----------------------------------------------------------" << endl;
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = {
        width,
        height,
        1
    };

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    EndSingleTimeCommands(commandBuffer);
    //cout << "VulkanCanvas::CopyBufferToImage(...) <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;
}

void VulkanCanvas::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    //cout << "VulkanCanvas::CopyBuffer(...) <----------------------------------------------------------" << endl;
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

    VkBufferCopy copyRegion = {};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    EndSingleTimeCommands(commandBuffer);
    //cout << "VulkanCanvas::CopyBuffer(...) <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;
}

void VulkanCanvas::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
    //cout << "VulkanCanvas::CreateBuffer(...) <----------------------------------------------------------" << endl;
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(m_logicalDevice, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_logicalDevice, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(m_logicalDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(m_logicalDevice, buffer, bufferMemory, 0);
    //cout << "VulkanCanvas::CreateBuffer(...) <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;
}

void VulkanCanvas::DrawFrame() {
    //cout << "VulkanCanvas::DrawFrame(...) <----------------------------------------------------------" << endl;
    vkWaitForFences(m_logicalDevice, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(m_logicalDevice, m_swapchain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        RecreateSwapchain();
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    UpdateUniformBuffer(NULL, imageIndex);

    if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
        vkWaitForFences(m_logicalDevice, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    }
    imagesInFlight[imageIndex] = inFlightFences[currentFrame];

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_commandBuffers[imageIndex];

    VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(m_logicalDevice, 1, &inFlightFences[currentFrame]);

    if (vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { m_swapchain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(m_presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
        framebufferResized = false;
        RecreateSwapchain();
    }
    else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    //cout << "VulkanCanvas::DrawFrame() <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;
}
void VulkanCanvas::MainLoop() {
    while (1) {
        DrawFrame();
    }

    vkDeviceWaitIdle(m_logicalDevice);
}
void VulkanCanvas::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
    //cout << "VulkanCanvas::TransitionImageLayout(...) <----------------------------------------------------------" << endl;
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else {
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    EndSingleTimeCommands(commandBuffer);
    //cout << "VulkanCanvas::TransitionImageLayout(...) <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;
}

VkCommandBuffer VulkanCanvas::BeginSingleTimeCommands() {
    //cout << "VulkanCanvas::BeginSingleTimeCommands(...) <----------------------------------------------------------" << endl;
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = m_commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(m_logicalDevice, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    //cout << "VulkanCanvas::TransitionImageLayout(...) <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;

    return commandBuffer;
}

void VulkanCanvas::EndSingleTimeCommands(VkCommandBuffer commandBuffer) {
    //cout << "VulkanCanvas::EndSingleTimeCommands(...) <----------------------------------------------------------" << endl;
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_graphicsQueue);

    vkFreeCommandBuffers(m_logicalDevice, m_commandPool, 1, &commandBuffer);
    //cout << "VulkanCanvas::EndSingleTimeCommands(...) <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;
}

VkFormat VulkanCanvas::FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
    cout << "VulkanCanvas::FindSupportedFormat(...) <----------------------------------------------------------" << endl;
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(m_physicalDevice, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported format!");
    cout << "VulkanCanvas::FindSupportedFormat(...) <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;
}

void VulkanCanvas::CreateDescriptorSetLayout() {
    cout << "VulkanCanvas::CreateDescriptorSetLayout(...) <----------------------------------------------------------" << endl;
    VkDescriptorSetLayoutBinding uboLayoutBinding = {};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.pImmutableSamplers = nullptr;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };
    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(m_logicalDevice, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
    cout << "VulkanCanvas::CreateDescriptorSetLayout() <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;
}

void VulkanCanvas::CreateGraphicsPipeline(const std::string& vertexShaderFile, const std::string& fragmentShaderFile)
{
    cout << "VulkanCanvas::CreateGraphicsPipeline(...) <----------------------------------------------------------" << endl;

    auto vertShaderCode = ReadFile("assets/shaders/vert.spv");
    auto fragShaderCode = ReadFile("assets/shaders/frag.spv");

    VkShaderModule vertShaderModule = CreateShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = CreateShaderModule(fragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();

    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)m_swapchainExtent.width;
    viewport.height = (float)m_swapchainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = { 0, 0 };
    scissor.extent = m_swapchainExtent;

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    //rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.cullMode = VK_CULL_MODE_NONE;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineDepthStencilStateCreateInfo depthStencil = {};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    //colorBlending.logicOpEnable = VK_TRUE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

    if (vkCreatePipelineLayout(m_logicalDevice, &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = m_pipelineLayout;
    pipelineInfo.renderPass = m_renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(m_logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_graphicsPipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    vkDestroyShaderModule(m_logicalDevice, fragShaderModule, nullptr);
    vkDestroyShaderModule(m_logicalDevice, vertShaderModule, nullptr);

    cout << "VulkanCanvas::CreateGraphicsPipeline(...) <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;
}

VkVertexInputBindingDescription Vertex::getBindingDescription() {
    cout << "VulkanCanvas::getBindingDescription() <----------------------------------------------------------" << endl;
    VkVertexInputBindingDescription bindingDescription = {};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    cout << "VulkanCanvas::getBindingDescription() <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;

    return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 3> Vertex::getAttributeDescriptions() {
    cout << "VulkanCanvas::getAttributeDescriptions() <----------------------------------------------------------" << endl;
    std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = {};

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, pos);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, color);

    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(Vertex, texCoord);
    cout << "VulkanCanvas::getAttributeDescriptions() <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;

    return attributeDescriptions;
}

std::vector<char> VulkanCanvas::ReadFile(const std::string& filename)
{
    cout << "VulkanCanvas::ReadFile(...) <----------------------------------------------------------" << endl;

    std::ifstream file(filename.c_str(), std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        std::stringstream ss;
        ss << "Failed to open file: " << filename;
        throw std::runtime_error(ss.str().c_str());
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();
    cout << "VulkanCanvas::ReadFile(...) <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;

    return buffer;
}

VkShaderModule VulkanCanvas::CreateShaderModule(const std::vector<char>& code) {
    cout << "VulkanCanvas::CreateShaderModule(...) <----------------------------------------------------------" << endl;

    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(m_logicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }
    cout << "VulkanCanvas::CreateShaderModule(...) <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;

    return shaderModule;
}

void VulkanCanvas::CreateFrameBuffers()
{
    cout << "VulkanCanvas::CreateFrameBuffers() <----------------------------------------------------------" << endl;

    m_swapchainFramebuffers.resize(m_swapchainImageViews.size());

    for (size_t i = 0; i < m_swapchainImageViews.size(); i++) {
        std::array<VkImageView, 2> attachments = {
            m_swapchainImageViews[i],
            m_depthImageView
        };

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = m_swapchainExtent.width;
        framebufferInfo.height = m_swapchainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(m_logicalDevice, &framebufferInfo, nullptr, &m_swapchainFramebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
    cout << "VulkanCanvas::CreateFrameBuffers() <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;
}

void VulkanCanvas::CreateCommandPool() {
    cout << "VulkanCanvas::CreateCommandPool() <----------------------------------------------------------" << endl;

    QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(m_physicalDevice);

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(m_logicalDevice, &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics command pool!");
    }
    cout << "VulkanCanvas::CreateCommandPool() <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;
}

void VulkanCanvas::CreateDepthResources() {
    cout << "VulkanCanvas::CreateDepthResources() <----------------------------------------------------------" << endl;
    VkFormat depthFormat = FindDepthFormat();

    CreateImage(m_swapchainExtent.width, m_swapchainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_depthImage, m_depthImageMemory);
    m_depthImageView = CreateImageView(m_depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
    cout << "VulkanCanvas::CreateDepthResources() <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;
}

void VulkanCanvas::CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
    //cout << "VulkanCanvas::CreateImage(...) <----------------------------------------------------------" << endl;
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(m_logicalDevice, &imageInfo, nullptr, &image) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(m_logicalDevice, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(m_logicalDevice, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(m_logicalDevice, image, imageMemory, 0);
    //cout << "VulkanCanvas::CreateImage(...) <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;
}

uint32_t VulkanCanvas::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    //cout << "VulkanCanvas::FindMemoryType(...) <----------------------------------------------------------" << endl;
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
    //cout << "VulkanCanvas::FindMemoryType(...) <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;
}

bool commandBufferCreated = false;

void VulkanCanvas::CreateCommandBuffers()
{
    //cout << "VulkanCanvas::CreateCommandBuffers() <----------------------------------------------------------" << endl;

    m_commandBuffers.resize(m_swapchainFramebuffers.size());

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)m_commandBuffers.size();

    if (!commandBufferCreated) {
        if (vkAllocateCommandBuffers(m_logicalDevice, &allocInfo, m_commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }
        else {
            commandBufferCreated = true;
        }
    }

    for (size_t i = 0; i < m_commandBuffers.size(); i++) {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(m_commandBuffers[i], &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_renderPass;
        renderPassInfo.framebuffer = m_swapchainFramebuffers[i];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = m_swapchainExtent;

        std::array<VkClearValue, 2> clearValues = {};
        clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
        clearValues[1].depthStencil = { 1.0f, 0 };

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(m_commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);

        for (int j = 0; j < m_objects.size(); j++) {
            if (m_objects[j]->isMD5File()) { // .md5
                if (((MyMD5Object*)m_objects[j])->drawObject) {
                    for (int k = 0; k < ((MyMD5Object*)m_objects[j])->myObjects.size(); k++) {
                        int tempCurFrame = ((MyMD5Object*)m_objects[j])->curFrame % ((MyMD5Object*)m_objects[j])->md5Model->m_Animation.m_iNumFrames;

                        VkBuffer vertexBuffers[] = { ((MyMD5Object*)m_objects[j])->myObjects[k]->vertexBuffers[tempCurFrame][((MyMD5Object*)m_objects[j])->myObjects[k]->curVertexBufferShown] };
                        VkDeviceSize offsets[] = { 0 };

                        vkCmdBindVertexBuffers(m_commandBuffers[i], 0, 1, vertexBuffers, offsets);

                        // controls what portion of indexbuffer to use
                        vkCmdBindIndexBuffer(m_commandBuffers[i], ((MyMD5Object*)m_objects[j])->myObjects[k]->indexBuffer, 0, VK_INDEX_TYPE_UINT32);

                        vkCmdBindDescriptorSets(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &((MyMD5Object*)m_objects[j])->myObjects[k]->descriptorSets[i], 0, nullptr);

                        vkCmdDrawIndexed(m_commandBuffers[i], static_cast<uint32_t>(((MyMD5Object*)m_objects[j])->myObjects[k]->indices.size()), 1, 0, 0, 0);

                        ((MyMD5Object*)m_objects[j])->myObjects[k]->curVertexBufferShown = (((MyMD5Object*)m_objects[j])->myObjects[k]->curVertexBufferShown + 1) % NUM_STAGING_BUFFER_FOR_VB;
                    }
                }

                md5FrameCounter--;
                if (md5FrameCounter <= 0) {
                    ((MyMD5Object*)m_objects[j])->curFrame = (((MyMD5Object*)m_objects[j])->curFrame + 1) % ((MyMD5Object*)m_objects[j])->md5Model->m_Animation.m_iNumFrames;
                    md5FrameCounter = MD5_FRAME_INTERVAL;
                }
            } // end of handling .md5
            else { // .obj
                if (((MyObject*)m_objects[j])->drawObject) {
                    // Draw object
                    VkBuffer vertexBuffers[] = { ((MyObject*)m_objects[j])->vertexBuffers[0][((MyObject*)m_objects[j])->curVertexBufferShown] };
                    VkDeviceSize offsets[] = { 0 };

                    vkCmdBindVertexBuffers(m_commandBuffers[i], 0, 1, vertexBuffers, offsets);

                    // controls what portion of indexbuffer to use
                    vkCmdBindIndexBuffer(m_commandBuffers[i], ((MyObject*)m_objects[j])->indexBuffer, 0, VK_INDEX_TYPE_UINT32);

                    vkCmdBindDescriptorSets(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &((MyObject*)m_objects[j])->descriptorSets[i], 0, nullptr);

                    vkCmdDrawIndexed(m_commandBuffers[i], static_cast<uint32_t>(((MyObject*)m_objects[j])->indices.size()), 1, 0, 0, 0);

                    ((MyObject*)m_objects[j])->curVertexBufferShown = (((MyObject*)m_objects[j])->curVertexBufferShown + 1) % NUM_STAGING_BUFFER_FOR_VB;
                }
            }
        }

        if (vkEndCommandBuffer(m_commandBuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }
    //cout << "VulkanCanvas::CreateCommandBuffers() <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;

}

VkSemaphoreCreateInfo VulkanCanvas::CreateSemaphoreCreateInfo() const noexcept
{
    cout << "VulkanCanvas::CreateSemaphoreCreateInfo() <----------------------------------------------------------" << endl;

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    cout << "VulkanCanvas::CreateSemaphoreCreateInfo() <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;
    return semaphoreInfo;
}

void VulkanCanvas::CreateSemaphores()
{
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    imagesInFlight.resize(m_swapchainImages.size(), VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(m_logicalDevice, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(m_logicalDevice, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(m_logicalDevice, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }
    cout << "VulkanCanvas::CreateSemaphores() <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;
}

void VulkanCanvas::CleanupSwapChain() {
    cout << "VulkanCanvas::CleanupSwapChain() <----------------------------------------------------------" << endl;
    vkDestroyImageView(m_logicalDevice, m_depthImageView, nullptr);
    vkDestroyImage(m_logicalDevice, m_depthImage, nullptr);
    vkFreeMemory(m_logicalDevice, m_depthImageMemory, nullptr);

    for (auto framebuffer : m_swapchainFramebuffers) {
        vkDestroyFramebuffer(m_logicalDevice, framebuffer, nullptr);
    }

    vkFreeCommandBuffers(m_logicalDevice, m_commandPool, static_cast<uint32_t>(m_commandBuffers.size()), m_commandBuffers.data());

    vkDestroyPipeline(m_logicalDevice, m_graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(m_logicalDevice, m_pipelineLayout, nullptr);
    vkDestroyRenderPass(m_logicalDevice, m_renderPass, nullptr);

    for (auto imageView : m_swapchainImageViews) {
        vkDestroyImageView(m_logicalDevice, imageView, nullptr);
    }

    vkDestroySwapchainKHR(m_logicalDevice, m_swapchain, nullptr);

    for (size_t i = 0; i < m_objects.size(); i++) {
        for (int j = 0; j < m_swapchainImages.size(); j++) {
            if (m_objects[i]->isObjFile()) {
                vkDestroyBuffer(m_logicalDevice, ((MyObject*)m_objects[i])->m_uniformBuffers[j], nullptr);
                vkFreeMemory(m_logicalDevice, ((MyObject*)m_objects[i])->m_uniformBuffersMemory[j], nullptr);
            }
            else {
                for (int k = 0; k < ((MyMD5Object*)m_objects[i])->myObjects.size(); k++) {
                    vkDestroyBuffer(m_logicalDevice, ((MyMD5Object*)m_objects[i])->myObjects[k]->m_uniformBuffers[j], nullptr);
                    vkFreeMemory(m_logicalDevice, ((MyMD5Object*)m_objects[i])->myObjects[k]->m_uniformBuffersMemory[j], nullptr);
                }
            }
        }
    }

    for (int i = 0; i < m_objects.size(); i++) {
        if (m_objects[i]->isObjFile()) {
            vkDestroyDescriptorPool(m_logicalDevice, ((MyObject*)m_objects[i])->descriptorPool, nullptr);
        }
        else {
            for (int j = 0; j < ((MyMD5Object*)m_objects[i])->myObjects.size(); j++) {
                vkDestroyDescriptorPool(m_logicalDevice, ((MyMD5Object*)m_objects[i])->myObjects[j]->descriptorPool, nullptr);
            }
        }
    }
    //vkDestroyDescriptorPool(device, descriptorPool1, nullptr);
    //vkDestroyDescriptorPool(device, descriptorPool2, nullptr);
    cout << "VulkanCanvas::CleanupSwapChain() <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;
}

void VulkanCanvas::RecreateSwapchain() {
    cout << "VulkanCanvas::RecreateSwapchain() <----------------------------------------------------------" << endl;

    vkDeviceWaitIdle(m_logicalDevice);
    CreateSwapChain(windowSize);
    CreateImageViews();
    CreateRenderPass();
    CreateGraphicsPipeline("..\assets\shaders\vert.spv", "..\assets\shaders\frag.spv");
    CreateDepthResources();
    CreateFrameBuffers();
    for (int i = 0; i < m_objects.size(); i++) {
        if (m_objects[i]->isObjFile()) {
            CreateUniformBuffers(m_objects[i], &((MyObject*)m_objects[i])->m_uniformBuffers, &((MyObject*)m_objects[i])->m_uniformBuffersMemory);
        }
        else {
            CreateUniformBuffers(m_objects[i], &m_objects[i]->m_uniformBuffers, &m_objects[i]->m_uniformBuffersMemory);
        }
    }
    CleanupSwapChain();

    for (int i = 0; i < m_objects.size(); i++) {
        if (m_objects[i]->isObjFile()) {
            CreateDescriptorPool(m_objects[i], &((MyObject*)m_objects[i])->descriptorPool);
        }
        else {
            for (int j = 0; j < ((MyMD5Object*)m_objects[i])->myObjects.size(); j++) {
                CreateDescriptorPool(m_objects[i], NULL);
            }
        }
    }
    for (int i = 0; i < m_objects.size(); i++) {
        if (m_objects[i]->isObjFile()) {
            CreateDescriptorSets(m_objects[i], &((MyObject*)m_objects[i])->descriptorPool, &((MyObject*)m_objects[i])->descriptorSets, &((MyObject*)m_objects[i])->textureImageView, &((MyObject*)m_objects[i])->textureSampler, &((MyObject*)m_objects[m_objects.size() - 1])->m_uniformBuffers);
        }
        else {
            for (int j = 0; j < ((MyMD5Object*)m_objects[i])->myObjects.size(); j++) {
                CreateDescriptorSets(m_objects[i], NULL, NULL, NULL, NULL, NULL);
            }
        }
    }
    CreateCommandBuffers();
    cout << "VulkanCanvas::RecreateSwapchain() <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;
}

VkSubmitInfo VulkanCanvas::CreateSubmitInfo(uint32_t imageIndex,
    VkPipelineStageFlags* waitStageFlags) const noexcept
{
    cout << "VulkanCanvas::CreateSubmitInfo(...) <----------------------------------------------------------" << endl;

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &m_imageAvailableSemaphore;
    submitInfo.pWaitDstStageMask = waitStageFlags;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_commandBuffers[imageIndex];

    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &m_renderFinishedSemaphore;
    return submitInfo;
    cout << "VulkanCanvas::CreateSubmitInfo(...) <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;
}

VkPresentInfoKHR VulkanCanvas::CreatePresentInfoKHR(uint32_t& imageIndex) const noexcept
{
    cout << "VulkanCanvas::CreatePresentInfoKHR(...) <----------------------------------------------------------" << endl;

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &m_renderFinishedSemaphore;

    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &m_swapchain;

    presentInfo.pImageIndices = &imageIndex;
    return presentInfo;
    cout << "VulkanCanvas::CreatePresentInfoKHR(...) <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;
}

void VulkanCanvas::OnPaint(wxPaintEvent& event)
{
    DrawFrame();
}

void VulkanCanvas::OnResize(wxSizeEvent& event)
{
    cout << "VulkanCanvas::OnResize(...) <----------------------------------------------------------" << endl;

    wxSize size = GetSize();
    if (size.GetWidth() == 0 || size.GetHeight() == 0) {
        return;
    }
    RecreateSwapchain();
    wxRect refreshRect(size);
    RefreshRect(refreshRect, false);
    cout << "VulkanCanvas::OnResize(...) <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;
}

void VulkanCanvas::OnPaintException(const std::string& msg)
{
    cout << "VulkanCanvas::OnPaintException(...) <----------------------------------------------------------" << endl;

    wxMessageBox(msg, "Vulkan Error");
    wxTheApp->ExitMainLoop();
    cout << "VulkanCanvas::OnPaintException(...) <XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;
}

glm::vec3 VulkanCanvas::getRightVector() {
    return glm::transpose(v_proj)[0];
}

void VulkanCanvas::SetCameraView(glm::vec3 eye, glm::vec3 lookat, glm::vec3 up) {
    glm::vec3 m_eye = std::move(eye);
    cameraX = m_eye.x;
    cameraY = m_eye.y;
    cameraZ = m_eye.z;

    glm::vec3 m_lookAt = std::move(lookat);
    viewingX = m_lookAt.x;
    viewingY = m_lookAt.y;
    viewingZ = m_lookAt.z;

    upVector = std::move(up);

    v_proj = glm::lookAt(m_eye, m_lookAt, upVector);
}

glm::vec3 VulkanCanvas::getViewDir() {
    return -glm::transpose(v_proj)[2];
}

void VulkanCanvas::ShowObject(string fileName, bool drawObject, bool showInSceneTree) {
    cout << "VulkanCanvas::ShowObject(): fileName: " << fileName << endl;

    // open .md5
    if (fileName.substr(fileName.length() - 8) == ".md5mesh") {
        MyMD5Object* newMyMD5Obj = new MyMD5Object(fileName);

        newMyMD5Obj->offset3D = glm::vec3(0.0f, 0.0f, 0.0f);

        int indexLastSlash = -1;
        for (int i = fileName.size() - 1; i >= 0 && indexLastSlash == -1; i--) {
            if (fileName[i] == '/') {
                indexLastSlash = i;
                break;
            }
        }

        m_objects.push_back(newMyMD5Obj);
        CreateTextureImage(newMyMD5Obj, fileName.substr(indexLastSlash + 1, fileName.substr(indexLastSlash + 1).length() - 8), NULL);
        CreateTextureImageView(newMyMD5Obj, NULL, NULL, NULL);
        CreateTextureSampler(newMyMD5Obj, NULL);
        CreateVertexBuffer(newMyMD5Obj, NULL, newMyMD5Obj->md5Model, NULL, NULL, NULL);
        CreateIndexBuffer(newMyMD5Obj, NULL, NULL, NULL, NULL);
        CreateUniformBuffers(newMyMD5Obj, &newMyMD5Obj->m_uniformBuffers, &newMyMD5Obj->m_uniformBuffersMemory);
        CreateDescriptorPool(newMyMD5Obj, NULL);
        CreateDescriptorSets(newMyMD5Obj, NULL, NULL, NULL, NULL, NULL);
        UpdateUniformBuffer(newMyMD5Obj, 0);
        UpdateUniformBuffer(newMyMD5Obj, 1);
        UpdateUniformBuffer(newMyMD5Obj, 2);
    }
    // open .obj
    else {
        MyObject* newMyObj = new MyObject(fileName);

        newMyObj->offset3D = glm::vec3(0.0f, 0.0f, 0.0f);

        newMyObj->drawObject = drawObject;

        CreateTextureImage(newMyObj, newMyObj->objFile, &newMyObj->textureImage);// we need 2 of this to load another object
        CreateTextureImageView(newMyObj, newMyObj->objFile, &newMyObj->textureImage, &newMyObj->textureImageView);
        CreateTextureSampler(newMyObj, &newMyObj->textureSampler);
        CreateVertexBuffer(newMyObj, newMyObj->objFile, NULL, &newMyObj->vertices, newMyObj->vertexBuffers[0], newMyObj->vertexBufferMemories[0]);
        CreateIndexBuffer(newMyObj, newMyObj->objFile, &newMyObj->indices, &newMyObj->indexBuffer, &newMyObj->indexBufferMemory);
        CreateUniformBuffers(newMyObj, &newMyObj->m_uniformBuffers, &newMyObj->m_uniformBuffersMemory);// we dont need 2 of this to load another object
        CreateDescriptorPool(newMyObj, &newMyObj->descriptorPool);
        CreateDescriptorSets(newMyObj, &newMyObj->descriptorPool,
            &newMyObj->descriptorSets, &newMyObj->textureImageView, &newMyObj->textureSampler, &newMyObj->m_uniformBuffers);

        m_objects.push_back(newMyObj);
        UpdateUniformBuffer(NULL, 0);
        UpdateUniformBuffer(NULL, 1);
        UpdateUniformBuffer(NULL, 2);
        CreateCommandBuffers();
        CreateSemaphores();

        newMyObj->createBoundingBox();
        //RecreateSwapchain();
    }
    initObject(m_objects.size() - 1);
    if (showInSceneTree) {
        int indexLastSlash = -1;
        for (int i = fileName.size() - 1; i >= 0 && indexLastSlash == -1; i--) {
            if (fileName[i] == '/') {
                indexLastSlash = i;
                break;
            }
        }
        minimalMainFrame->objHierarchy->addObjItem(fileName.substr(indexLastSlash + 1), m_objects.size() - 1 - NUM_STARTING_OBJECTS);
        m_objects.at(m_objects.size() - 1)->sceneTreeName = fileName.substr(indexLastSlash + 1);
    }
}

void VulkanCanvas::initCamera() {
    cameraX = CAMERA_X;
    cameraY = CAMERA_Y;
    cameraZ = CAMERA_Z;

    viewingX = LOOKAT_X;
    viewingY = LOOKAT_Y;
    viewingZ = LOOKAT_Z;

    cameraXBeforeDrag = cameraX;
    cameraYBeforeDrag = cameraY;
    cameraZBeforeDrag = cameraZ;

    cameraXBeforeRightDrag = cameraX;
    cameraYBeforeRightDrag = cameraY;
    cameraZBeforeRightDrag = cameraZ;

    cameraDirection.x = cameraX - viewingX;
    cameraDirection.y = cameraY - viewingY;
    cameraDirection.z = cameraZ - viewingZ;

    cameraDirection = glm::normalize(cameraDirection);

    printCameraInfo();
}

void VulkanCanvas::initObject(int index) {
    m_objects.at(index)->offset3D = glm::vec3(0.0f, 0.0f, 0.0f);
    m_objects.at(index)->offset3DRot = glm::vec3(0.0f, 0.0f, 0.0f);
    m_objects.at(index)->quaternion = glm::quat(0.0f, 0.0f, 0.0f, 0.0f);

    m_objects.at(index)->translationVelocity = glm::vec3(0.0f, 0.0f, 0.0f);
    m_objects.at(index)->angularVelocity = glm::vec3(0.0f, 0.0f, 0.0f);
    m_objects.at(index)->acceleration = glm::vec3(0.0f, -0.01f, 0.0f);
    m_objects.at(index)->objectDensity = 1.0f;

    if (m_objects.at(index)->isObjFile()) {
        ((MyObject* )m_objects.at(index))->createBoundingBox();
    }
}

void VulkanCanvas::printCameraInfo() {
    cout << "printCameraInfo(): -------------------------------" << endl;

    cout << "cameraX: " << cameraX << endl;
    cout << "cameraY: " << cameraY << endl;
    cout << "cameraZ: " << cameraZ << endl;

    cout << "viewingX: " << viewingX << endl;
    cout << "viewingY: " << viewingY << endl;
    cout << "viewingZ: " << viewingZ << endl;

    cout << "cameraXBeforeDrag: " << cameraXBeforeDrag << endl;
    cout << "cameraYBeforeDrag: " << cameraYBeforeDrag << endl;
    cout << "cameraZBeforeDrag: " << cameraZBeforeDrag << endl;

    cout << "cameraXBeforeRightDrag: " << cameraXBeforeRightDrag << endl;
    cout << "cameraYBeforeRightDrag: " << cameraYBeforeRightDrag << endl;
    cout << "cameraZBeforeRightDrag: " << cameraZBeforeRightDrag << endl;

    cout << "cameraDirection.x: " << cameraDirection.x << endl;
    cout << "cameraDirection.y: " << cameraDirection.y << endl;
    cout << "cameraDirection.z: " << cameraDirection.z << endl;

    cout << "printCameraInfo(): XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;
}
