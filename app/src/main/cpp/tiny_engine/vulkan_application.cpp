#include "vulkan_application.h"

#include <vulkan/vulkan_android.h>
#include <set>
#include <string>

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
    vkDestroyDevice(device_, nullptr);
    vkDestroySurfaceKHR(instance_, surface_, nullptr);
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

void VulkanApplication::CreateSurface() {
    VkAndroidSurfaceCreateInfoKHR create_info;
    create_info.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
    create_info.pNext = nullptr;
    create_info.flags = 0;
    create_info.window = static_cast<ANativeWindow *>(native_window_);

    if (vkCreateAndroidSurfaceKHR(instance_, &create_info, nullptr,
                                  &surface_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create surface!");
    }
}

void VulkanApplication::CreateDevice() {
    physical_device_ = PickPhysicalDevice(instance_, device_extensions_, surface_);
    QueueFamilyIndices indices = FindQueueFamilies(physical_device_, surface_);

    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    std::set<uint32_t> unique_queue_families = {(uint32_t) indices.graphics_family,
                                                (uint32_t) indices.present_family};
    float queue_priority = 1.0f;
    for (uint32_t queue_family : unique_queue_families) {
        VkDeviceQueueCreateInfo queue_create_info{};
        queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info.queueFamilyIndex = queue_family;
        queue_create_info.queueCount = 1;
        queue_create_info.pQueuePriorities = &queue_priority;
        queue_create_infos.push_back(queue_create_info);
    }

    VkPhysicalDeviceFeatures device_features{};
    VkDeviceCreateInfo device_create_info{};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pQueueCreateInfos = queue_create_infos.data();
    device_create_info.queueCreateInfoCount = static_cast<uint32_t >(queue_create_infos.size());
    device_create_info.pEnabledFeatures = &device_features;
    device_create_info.enabledLayerCount = 0;
    device_create_info.ppEnabledLayerNames = nullptr;
    device_create_info.enabledExtensionCount = static_cast<uint32_t>(device_extensions_.size());
    device_create_info.ppEnabledExtensionNames = device_extensions_.data();

    if (vkCreateDevice(physical_device_, &device_create_info, nullptr, &device_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }

    vkGetDeviceQueue(device_, indices.graphics_family, 0, &graphics_queue_);
    vkGetDeviceQueue(device_, indices.present_family, 0, &present_queue_);

    if (graphics_queue_ == VK_NULL_HANDLE || present_queue_ == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to get graphics or present queue failed!");
    }
}

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

/********* helper method ***********/

VkPhysicalDevice VulkanApplication::PickPhysicalDevice(VkInstance instance,
                                                       const std::vector<const char *> &extensions,
                                                       VkSurfaceKHR surface) {
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(instance, &device_count, nullptr);

    if (device_count == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(instance, &device_count, devices.data());

    VkPhysicalDevice physical_device;
    for (const auto &device : devices) {
        QueueFamilyIndices indices = FindQueueFamilies(device, surface);
        if (!indices.IsComplete()) {
            continue;
        }

        bool extensions_supported = CheckDeviceExtensionSupport(device, extensions);
        if (!extensions_supported) {
            continue;
        }

        SwapChainSupportDetails details = QuerySwapChainSupport(device, surface);
        if (details.formats.empty() || details.present_modes.empty()) {
            continue;
        }

        VkPhysicalDeviceFeatures supported_features;
        vkGetPhysicalDeviceFeatures(device, &supported_features);
        if (!supported_features.samplerAnisotropy) {
            continue;
        }
        physical_device = device;
    }

    if (physical_device == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
    return physical_device;
}

QueueFamilyIndices
VulkanApplication::FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
    QueueFamilyIndices indices;

    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);

    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());

    int i = 0;
    for (const auto &queue_family : queue_families) {
        if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphics_family = i;
        }

        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);
        if (present_support) {
            indices.present_family = i;
        }

        if (indices.IsComplete()) {
            break;
        }

        i++;
    }

    return indices;
}

bool VulkanApplication::CheckDeviceExtensionSupport(VkPhysicalDevice device,
                                                    const std::vector<const char *> &extensions) {
    uint32_t extension_count;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);

    std::vector<VkExtensionProperties> available_extensions(extension_count);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count,
                                         available_extensions.data());

    std::set<std::string> required_extensions(extensions.begin(), extensions.end());

    for (const auto &extension : available_extensions) {
        required_extensions.erase(extension.extensionName);
    }

    return required_extensions.empty();
}


SwapChainSupportDetails
VulkanApplication::QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    uint32_t format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, nullptr);

    if (format_count != 0) {
        details.formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count,
                                             details.formats.data());
    }

    uint32_t present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count,
                                              nullptr);

    if (present_mode_count != 0) {
        details.present_modes.resize(present_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count,
                                                  details.present_modes.data());
    }
    return details;
}

} // namespace tiny_engine