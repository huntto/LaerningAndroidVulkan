#ifndef ANDROID_VULKAN_CUBE_APPLICATION_H
#define ANDROID_VULKAN_CUBE_APPLICATION_H

#include <vulkan_application.h>

#include <vulkan/vulkan_android.h>
#include <vector>
#define GLM_FORCE_RADIANS
#define GLM_LANG_STL11_FORCED
#include <glm/glm.hpp>

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

struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

class CubeApplication : public tiny_engine::VulkanApplication {
public:
    CubeApplication(void *native_window,
                    std::vector<char> vert_shader_code,
                    std::vector<char> frag_shader_code);

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

private:
    std::vector<Vertex> vertices_ = {
            // front
            {{-1.0f, -1.0f, 1.0f},  {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
            {{1.0f,  -1.0f, 1.0f},  {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
            {{1.0f,  1.0f,  1.0f},  {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
            {{-1.0f, 1.0f,  1.0f},  {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},

            // back
            {{-1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
            {{1.0f,  -1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
            {{1.0f,  1.0f,  -1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
            {{-1.0f, 1.0f,  -1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},

            // left
            {{-1.0f, 1.0f,  -1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
            {{-1.0f, -1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
            {{-1.0f, -1.0f, 1.0f},  {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
            {{-1.0f, 1.0f,  1.0f},  {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},

            // right
            {{1.0f,  1.0f,  -1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
            {{1.0f,  -1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
            {{1.0f,  -1.0f, 1.0f},  {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
            {{1.0f,  1.0f,  1.0f},  {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},

            // top
            {{-1.0f, 1.0f,  1.0f},  {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
            {{1.0f,  1.0f,  1.0f},  {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
            {{1.0f,  1.0f,  -1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
            {{-1.0f, 1.0f,  -1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},

            // bottom
            {{-1.0f, -1.0f, 1.0f},  {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
            {{1.0f,  -1.0f, 1.0f},  {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
            {{1.0f,  -1.0f, -1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
            {{-1.0f, -1.0f, -1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
    };
    std::vector<uint16_t> indices_ = {0, 1, 2, 2, 3, 0,
                                      4, 6, 5, 4, 7, 6,
                                      8, 9, 10, 10, 11, 8,
                                      12, 14, 13, 12, 15, 14,
                                      16, 17, 18, 18, 19, 16,
                                      20, 22, 21, 20, 23, 22};
    size_t current_frame_ = 0;

    VkImage texture_image_;
    VkDeviceMemory texture_image_memory_;
    VkImageView texture_image_view_;
    VkSampler texture_sampler_;

    UniformBufferObject ubo_;
};


#endif //ANDROID_VULKAN_CUBE_APPLICATION_H
