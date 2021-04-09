#ifndef TINY_ENGINE_VULKAN_APPLICATION_H
#define TINY_ENGINE_VULKAN_APPLICATION_H

#include <vulkan/vulkan.h>
#include <string>
#include <vector>

namespace tiny_engine {

class VulkanApplication {
public:
    virtual void Init();

    virtual void Draw();

    virtual void Cleanup();

protected:
    virtual void CreateInstance();

    virtual void CreateDebugMessenger();

    virtual void CreateSurface();

    virtual void CreateDevice();

    virtual void CreateSwapChain();

    virtual void CreateGraphicsPipeline();

    virtual void CreateShaderModules();

    virtual void CreateRenderPass();

    virtual void CreateFramebuffers();

    virtual void CreateCommandPool();

    virtual void CreateCommandBuffers();

    virtual void CreateSyncObjects();

    virtual void CreateVertexBuffer();

    virtual void CreateIndexBuffer();

    virtual void CreateDescriptorSetLayout();

    virtual void CreateUniformBuffers();

    virtual void CreateDescriptorPool();

    virtual void CreateDescriptorSets();

    virtual void CreateTextureImage();

    virtual void CreateTextureImageView();

    virtual void CreateTextureSampler();

    virtual void CreateDepthResources();

    virtual void DestroyDebugMessenger();

protected:
    std::string application_name_;
    std::vector<const char *> extensions_;
    std::vector<const char *> layers_;
    VkInstance instance_ = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debug_messenger_ = VK_NULL_HANDLE;
};

} //namespace tiny_engine

#endif //TINY_ENGINE_VULKAN_APPLICATION_H
