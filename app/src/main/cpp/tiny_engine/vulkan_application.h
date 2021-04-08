#ifndef TINY_ENGINE_VULKAN_APPLICATION_H
#define TINY_ENGINE_VULKAN_APPLICATION_H

namespace tiny_engine {

class VulkanApplication {
public:
    virtual void Init();

    virtual void Draw();

    virtual void Cleanup();

protected:
    virtual void CreateInstance();

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
};

} //namespace tiny_engine

#endif //TINY_ENGINE_VULKAN_APPLICATION_H
