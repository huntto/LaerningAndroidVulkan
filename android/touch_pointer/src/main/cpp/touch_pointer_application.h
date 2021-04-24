#ifndef ANDROID_VULKAN_TOUCH_POINTER_APPLICATION_H
#define ANDROID_VULKAN_TOUCH_POINTER_APPLICATION_H

#include <vulkan_application.h>

#include <vulkan/vulkan_android.h>
#include <vector>
#define GLM_FORCE_RADIANS
#define GLM_LANG_STL11_FORCED
#include <glm/glm.hpp>
#include <array>
#include <chrono>

struct Vertex {
    glm::vec3 pos = glm::vec3(0.0f, 0.0f, 100.f);
    glm::vec3 color;
    float size;

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
        attribute_descriptions[2].format = VK_FORMAT_R32_SFLOAT;
        attribute_descriptions[2].offset = offsetof(Vertex, size);
        return attribute_descriptions;
    }

    bool operator==(const Vertex &other) const {
        return pos == other.pos && color == other.color && size == other.size;
    }
};

struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

enum Action {
    DOWN = 0,
    MOVE,
    UP
};

class TouchPointerApplication : public tiny_engine::VulkanApplication {
public:
    TouchPointerApplication(void *native_window,
                            std::vector<char> vert_shader_code,
                            std::vector<char> frag_shader_code);

    virtual void Draw() override;

    void UpdatePointer(int index, float x, float y, float size, Action action);

protected:
    virtual void CreateDescriptorSetLayout() override;

    virtual void CreateVertexBuffer() override;

    virtual void CreateUniformBuffers() override;

    virtual void CreateDescriptorPool() override;

    virtual void CreateDescriptorSets() override;

    virtual void CreateCommandBuffers() override;

    void Update(uint32_t current_image);

private:
    UniformBufferObject ubo_;
    size_t current_frame_ = 0;
    std::array<Vertex, 20> vertices_;
};


#endif //ANDROID_VULKAN_TOUCH_POINTER_APPLICATION_H
