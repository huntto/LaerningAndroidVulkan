#ifndef ANDROID_VULKAN_MODEL_APPLICATION_H
#define ANDROID_VULKAN_MODEL_APPLICATION_H

#include <vulkan_application.h>

#include <vulkan/vulkan_android.h>
#include <vector>
#define GLM_FORCE_RADIANS
#define GLM_LANG_STL11_FORCED
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 tex_coord;

    static std::vector<VkVertexInputBindingDescription> GetBindingDescription() {
        VkVertexInputBindingDescription binding_description{};
        binding_description.binding = 0;
        binding_description.stride = sizeof(Vertex);
        binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return {binding_description};
    }

    static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attribute_descriptions(3);
        attribute_descriptions[0].binding = 0;
        attribute_descriptions[0].location = 0;
        attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attribute_descriptions[0].offset = offsetof(Vertex, pos);
        attribute_descriptions[1].binding = 0;
        attribute_descriptions[1].location = 1;
        attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attribute_descriptions[1].offset = offsetof(Vertex, color);
        attribute_descriptions[2].binding = 0;
        attribute_descriptions[2].location = 2;
        attribute_descriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attribute_descriptions[2].offset = offsetof(Vertex, tex_coord);
        return attribute_descriptions;
    }

    bool operator==(const Vertex &other) const {
        return pos == other.pos && color == other.color && tex_coord == other.tex_coord;
    }
};

namespace std {
template<>
struct hash<Vertex> {
    size_t operator()(Vertex const &vertex) const {
        return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
               (hash<glm::vec2>()(vertex.tex_coord) << 1);
    }
};
}

struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

class ModelApplication : public tiny_engine::VulkanApplication {
public:
    ModelApplication(void *native_window,
                     std::vector<char> vert_shader_code,
                     std::vector<char> frag_shader_code);

    virtual void Init() override;

    virtual void Draw() override;

    virtual void Cleanup() override;

    virtual void Rotate(float radius, float x, float y, float z);

protected:
    virtual void CreateDescriptorSetLayout() override;

    virtual void CreateTextureImage() override;

    virtual void CreateTextureImageView() override;

    virtual void CreateTextureSampler() override;

    virtual void CreateVertexBuffer() override;

    virtual void CreateIndexBuffer() override;

    virtual void CreateUniformBuffers() override;

    virtual void CreateDescriptorPool() override;

    virtual void CreateDescriptorSets() override;

    virtual void CreateCommandBuffers() override;

private:
    void UpdateUniformBuffer(uint32_t current_image);

    void CreateModel();

private:
    std::vector<Vertex> vertices_;
    std::vector<uint16_t> indices_;
    size_t current_frame_ = 0;

    VkImage texture_image_;
    VkDeviceMemory texture_image_memory_;
    VkImageView texture_image_view_;
    VkSampler texture_sampler_;

    UniformBufferObject ubo_;
};


#endif //ANDROID_VULKAN_MODEL_APPLICATION_H
