#include "vulkan_application.h"

#include "log.h"

namespace tiny_engine {

void VulkanApplication::Init() {
    CreateInstance();
    CreateDebugMessenger();
    CreateSurface();
    CreateDevice();
    CreateSwapChain();
    CreateGraphicsPipeline();
    CreateShaderModules();
    CreateRenderPass();
    CreateFramebuffers();
    CreateCommandPool();
    CreateCommandBuffers();
    CreateSyncObjects();
    CreateVertexBuffer();
    CreateIndexBuffer();
    CreateDescriptorSetLayout();
    CreateUniformBuffers();
    CreateDescriptorPool();
    CreateDescriptorSets();
    CreateTextureImage();
    CreateTextureImageView();
    CreateTextureSampler();
    CreateDepthResources();
}

void VulkanApplication::Draw() {}

void VulkanApplication::Cleanup() {
    DestroyDebugMessenger();
    vkDestroyInstance(instance_, nullptr);
}

void VulkanApplication::CreateInstance() {
    VkApplicationInfo app_info{};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = application_name_.c_str();
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "Tiny Engine";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;

    create_info.enabledExtensionCount = extensions_.size();
    create_info.ppEnabledExtensionNames = extensions_.data();

    create_info.enabledLayerCount = layers_.size();
    create_info.ppEnabledLayerNames = layers_.data();

    if (vkCreateInstance(&create_info, nullptr, &instance_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }
}

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
        VkDebugUtilsMessageTypeFlagsEXT message_type,
        const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
        void *user_data) {
    LOGI("Validation layer: %s", callback_data->pMessage);
    return VK_FALSE;
}

void VulkanApplication::CreateDebugMessenger() {
    if (layers_.empty()) return;

    VkDebugUtilsMessengerCreateInfoEXT create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
                                  | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                                  | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                              | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                              | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    create_info.pfnUserCallback = DebugCallback;
    create_info.pUserData = nullptr;
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
            instance_,
            "vkCreateDebugUtilsMessengerEXT");
    if (func(instance_, &create_info, nullptr, &debug_messenger_) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create debug messenger!");
    }
}

void VulkanApplication::CreateSurface() {}

void VulkanApplication::CreateDevice() {}

void VulkanApplication::CreateSwapChain() {}

void VulkanApplication::CreateGraphicsPipeline() {}

void VulkanApplication::CreateShaderModules() {}

void VulkanApplication::CreateRenderPass() {}

void VulkanApplication::CreateFramebuffers() {}

void VulkanApplication::CreateCommandPool() {}

void VulkanApplication::CreateCommandBuffers() {}

void VulkanApplication::CreateSyncObjects() {}

void VulkanApplication::CreateVertexBuffer() {}

void VulkanApplication::CreateIndexBuffer() {}

void VulkanApplication::CreateDescriptorSetLayout() {}

void VulkanApplication::CreateUniformBuffers() {}

void VulkanApplication::CreateDescriptorPool() {}

void VulkanApplication::CreateDescriptorSets() {}

void VulkanApplication::CreateTextureImage() {}

void VulkanApplication::CreateTextureImageView() {}

void VulkanApplication::CreateTextureSampler() {}

void VulkanApplication::CreateDepthResources() {}

void VulkanApplication::DestroyDebugMessenger() {
    if (debug_messenger_ == VK_NULL_HANDLE) return;
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance_,
                                                                            "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance_, debug_messenger_, nullptr);
    }
}

} // namespace tiny_engine