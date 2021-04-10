#include "vulkan_application.h"

#include <vulkan/vulkan_android.h>
#include <android/native_window.h>
#include <set>
#include <string>

#include "log.h"

namespace tiny_engine {

void VulkanApplication::Init() {
    CreateInstance();
    CreateDebugMessenger();
    CreateSurface();
    CreateDevice();
    CreateSwapchain();
    CreateSwapchainImageViews();
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
    for (auto image_view : swapchain_image_views_) {
        vkDestroyImageView(device_, image_view, nullptr);
    }
    swapchain_image_views_.clear();

    vkDestroySwapchainKHR(device_, swapchain_, nullptr);
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

void VulkanApplication::CreateSwapchain() {
    SwapChainSupportDetails support_details = QuerySwapChainSupport(physical_device_, surface_);
    VkSurfaceFormatKHR surface_format = ChooseSwapSurfaceFormat(support_details.formats);
    VkPresentModeKHR present_mode = ChooseSwapPresentMode(support_details.present_modes);
    VkExtent2D extent = ChooseSwapExtent(support_details.capabilities);

    uint32_t image_count = support_details.capabilities.minImageCount + 1;
    if (support_details.capabilities.maxImageCount > 0
        && image_count > support_details.capabilities.maxImageCount) {
        image_count = support_details.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = surface_;

    create_info.minImageCount = image_count;
    create_info.imageFormat = surface_format.format;
    create_info.imageColorSpace = surface_format.colorSpace;
    create_info.imageExtent = extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = FindQueueFamilies(physical_device_, surface_);
    uint32_t queue_family_indices[] = {
            static_cast<uint32_t>(indices.graphics_family),
            static_cast<uint32_t>(indices.present_family)
    };

    if (indices.graphics_family != indices.present_family) {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = queue_family_indices;
    } else {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    create_info.preTransform = support_details.capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
    create_info.presentMode = present_mode;
    create_info.clipped = VK_TRUE;

    create_info.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(device_, &create_info, nullptr, &swapchain_) != VK_SUCCESS
        || swapchain_ == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(device_, swapchain_, &image_count, nullptr);
    swapchain_images_.resize(image_count);
    vkGetSwapchainImagesKHR(device_, swapchain_, &image_count, swapchain_images_.data());

    swapchain_image_format_ = surface_format.format;
    swapchain_extent_ = extent;
}

void VulkanApplication::CreateSwapchainImageViews() {
    swapchain_image_views_.resize(swapchain_images_.size());
    for (size_t i = 0; i < swapchain_images_.size(); i++) {
        swapchain_image_views_[i] = CreateImageView(device_,
                                                    swapchain_images_[i],
                                                    swapchain_image_format_,
                                                    VK_IMAGE_ASPECT_COLOR_BIT);
    }
}

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

VkSurfaceFormatKHR VulkanApplication::ChooseSwapSurfaceFormat(
        const std::vector<VkSurfaceFormatKHR> &available_formats) {
    for (const auto &available_format : available_formats) {
        if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB
            && available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return available_format;
        }
    }

    return available_formats[0];
}

VkPresentModeKHR VulkanApplication::ChooseSwapPresentMode(
        const std::vector<VkPresentModeKHR> &available_present_modes) {
    for (const auto &available_present_mode : available_present_modes) {
        if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return available_present_mode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanApplication::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) {
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    } else {
        uint32_t width = ANativeWindow_getWidth(static_cast<ANativeWindow *>(native_window_));
        uint32_t height = ANativeWindow_getHeight(static_cast<ANativeWindow *>(native_window_));
        VkExtent2D actual_extent = {width, height};

        actual_extent.width = std::max(capabilities.minImageExtent.width,
                                       std::min(capabilities.maxImageExtent.width,
                                                actual_extent.width));
        actual_extent.height = std::max(capabilities.minImageExtent.height,
                                        std::min(capabilities.maxImageExtent.height,
                                                 actual_extent.height));

        return actual_extent;
    }
}

VkImageView VulkanApplication::CreateImageView(VkDevice device, VkImage image, VkFormat format,
                                               VkImageAspectFlags aspect_flags) {
    VkImageViewCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    create_info.image = image;
    create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    create_info.format = format;
    create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    create_info.subresourceRange.baseMipLevel = 0;
    create_info.subresourceRange.levelCount = 1;
    create_info.subresourceRange.baseArrayLayer = 0;
    create_info.subresourceRange.layerCount = 1;
    create_info.subresourceRange.aspectMask = aspect_flags;

    VkImageView image_view = VK_NULL_HANDLE;
    if (vkCreateImageView(device, &create_info, nullptr, &image_view) != VK_SUCCESS ||
        image_view == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to create texture image view!");
    }
    return image_view;
}

} // namespace tiny_engine