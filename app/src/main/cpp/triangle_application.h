#ifndef TINY_ENGINE_TRIANGLE_APPLICATION_H
#define TINY_ENGINE_TRIANGLE_APPLICATION_H

#include "tiny_engine/vulkan_application.h"

#include <vulkan/vulkan_android.h>

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
    }
};


#endif //TINY_ENGINE_TRIANGLE_APPLICATION_H
