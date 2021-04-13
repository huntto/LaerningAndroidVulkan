#include "triangle_application.h"
#include "tiny_engine/log.h"

TriangleApplication::TriangleApplication(void *native_window, std::vector<char> vert_shader_code,
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

void TriangleApplication::CreateVertexBuffer() {
    VkDeviceSize buffer_size = sizeof(vertices_[0]) * vertices_.size();

    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    CreateBuffer(physical_device_,
                 device_,
                 buffer_size,
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 staging_buffer,
                 staging_buffer_memory);


    void *data;
    vkMapMemory(device_, staging_buffer_memory, 0, buffer_size, 0, &data);
    memcpy(data, vertices_.data(), (size_t) buffer_size);
    vkUnmapMemory(device_, staging_buffer_memory);

    CreateBuffer(physical_device_,
                 device_,
                 buffer_size,
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 vertex_buffer_,
                 vertex_buffer_memory_);

    CopyBuffer(device_,
               command_pool_,
               graphics_queue_,
               staging_buffer,
               vertex_buffer_,
               buffer_size);

    vkDestroyBuffer(device_, staging_buffer, nullptr);
    vkFreeMemory(device_, staging_buffer_memory, nullptr);
}

void TriangleApplication::CreateIndexBuffer() {
    VkDeviceSize buffer_size = sizeof(indices_[0]) * indices_.size();
    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    CreateBuffer(physical_device_,
                 device_,
                 buffer_size,
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 staging_buffer,
                 staging_buffer_memory);

    void *data;
    vkMapMemory(device_, staging_buffer_memory, 0, buffer_size, 0, &data);
    memcpy(data, indices_.data(), (size_t) buffer_size);
    vkUnmapMemory(device_, staging_buffer_memory);

    CreateBuffer(physical_device_,
                 device_,
                 buffer_size,
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 index_buffer_,
                 index_buffer_memory_);

    CopyBuffer(device_,
               command_pool_,
               graphics_queue_,
               staging_buffer,
               index_buffer_,
               buffer_size);

    vkDestroyBuffer(device_, staging_buffer, nullptr);
    vkFreeMemory(device_, staging_buffer_memory, nullptr);
}

void TriangleApplication::CreateUniformBuffers() {
    VkDeviceSize buffer_size = sizeof(UniformBufferObject);

    uniform_buffers_.resize(swapchain_images_.size());
    uniform_buffers_memory_.resize(swapchain_images_.size());

    for (size_t i = 0; i < swapchain_images_.size(); i++) {
        CreateBuffer(physical_device_,
                     device_,
                     buffer_size,
                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     uniform_buffers_[i],
                     uniform_buffers_memory_[i]);
    }
}

void TriangleApplication::CreateDescriptorPool() {
    std::vector<VkDescriptorPoolSize> pool_sizes(1);
    pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pool_sizes[0].descriptorCount = static_cast<uint32_t>(swapchain_images_.size());

    VkDescriptorPoolCreateInfo pool_create_info{};
    pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_create_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
    pool_create_info.pPoolSizes = pool_sizes.data();
    pool_create_info.maxSets = static_cast<uint32_t>(swapchain_images_.size());
    if (vkCreateDescriptorPool(device_, &pool_create_info, nullptr, &descriptor_pool_) !=
        VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}