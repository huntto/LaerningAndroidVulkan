#ifndef TINY_ENGINE_TRIANGLE_APPLICATION_H
#define TINY_ENGINE_TRIANGLE_APPLICATION_H

#include "tiny_engine/vulkan_application.h"

#include <vulkan/vulkan_android.h>
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

class TriangleApplication : public tiny_engine::VulkanApplication {
public:
    TriangleApplication(void *native_window,
                        std::vector<char> vert_shader_code,
                        std::vector<char> frag_shader_code) {
        layers_ = {
                "VK_LAYER_KHRONOS_validation"
        };
        extensions_ = {
                VK_KHR_SURFACE_EXTENSION_NAME,
                VK_KHR_ANDROID_SURFACE_EXTENSION_NAME,
                VK_EXT_DEBUG_UTILS_EXTENSION_NAME
        };
        application_name_ = "Triangle";
        native_window_ = native_window;
        vert_shader_code_ = vert_shader_code;
        frag_shader_code_ = frag_shader_code;
        binding_descriptions_ = Vertex::GetBindingDescription();
        attribute_descriptions_ = Vertex::GetAttributeDescriptions();
    }
};


#endif //TINY_ENGINE_TRIANGLE_APPLICATION_H
