#ifndef TINY_ENGINE_TRIANGLE_APPLICATION_H
#define TINY_ENGINE_TRIANGLE_APPLICATION_H

#include "tiny_engine/vulkan_application.h"

#include <vulkan/vulkan_android.h>
#include <vector>
#define GLM_FORCE_RADIANS
#define GLM_LANG_STL11_FORCED
#include <glm/glm.hpp>

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;

    static std::vector<VkVertexInputBindingDescription> GetBindingDescription() {
        VkVertexInputBindingDescription binding_description{};
        binding_description.binding = 0;
        binding_description.stride = sizeof(Vertex);
        binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return {binding_description};
    }

    static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attribute_descriptions(2);
        attribute_descriptions[0].binding = 0;
        attribute_descriptions[0].location = 0;
        attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attribute_descriptions[0].offset = offsetof(Vertex, pos);
        attribute_descriptions[1].binding = 0;
        attribute_descriptions[1].location = 1;
        attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attribute_descriptions[1].offset = offsetof(Vertex, color);
        return attribute_descriptions;
    }

    bool operator==(const Vertex &other) const {
        return pos == other.pos && color == other.color;
    }
};

struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

class TriangleApplication : public tiny_engine::VulkanApplication {
public:
    TriangleApplication(void *native_window,
                        std::vector<char> vert_shader_code,
                        std::vector<char> frag_shader_code);

    virtual void Draw() override;

protected:
    virtual void CreateDescriptorSetLayout() override;

    virtual void CreateVertexBuffer() override;

    virtual void CreateIndexBuffer() override;

    virtual void CreateUniformBuffers() override;

    virtual void CreateDescriptorPool() override;

    virtual void CreateDescriptorSets() override;

    virtual void CreateCommandBuffers() override;

private:
    std::vector<Vertex> vertices_ = {
            {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f,  -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
            {{0.5f,  0.5f,  0.0f}, {0.0f, 0.0f, 1.0f}},
            {{-0.5f, 0.5f,  0.0f}, {1.0f, 1.0f, 1.0f}}
    };
    std::vector<uint16_t> indices_ = {0, 1, 2, 2, 3, 0};
};


#endif //TINY_ENGINE_TRIANGLE_APPLICATION_H
