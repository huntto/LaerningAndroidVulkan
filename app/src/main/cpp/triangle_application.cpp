#include "triangle_application.h"

#include <array>

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
    max_frames_in_flight_ = 2;
}

void TriangleApplication::Draw() {}

void TriangleApplication::CreateDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding ubo_layout_binding{};
    ubo_layout_binding.binding = 0;
    ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubo_layout_binding.descriptorCount = 1;
    ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    ubo_layout_binding.pImmutableSamplers = nullptr;

    std::array<VkDescriptorSetLayoutBinding, 1> bindings = {ubo_layout_binding};

    VkDescriptorSetLayoutCreateInfo layout_create_info{};
    layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_create_info.bindingCount = static_cast<uint32_t>(bindings.size());
    layout_create_info.pBindings = bindings.data();
    if (vkCreateDescriptorSetLayout(device_, &layout_create_info, nullptr,
                                    &descriptor_set_layout_) != VK_SUCCESS ||
        descriptor_set_layout_ == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
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
    std::array<VkDescriptorPoolSize, 1> pool_sizes;
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

void TriangleApplication::CreateDescriptorSets() {
    std::vector<VkDescriptorSetLayout> layouts(swapchain_images_.size(), descriptor_set_layout_);
    VkDescriptorSetAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = descriptor_pool_;
    alloc_info.descriptorSetCount = static_cast<uint32_t>(swapchain_images_.size());
    alloc_info.pSetLayouts = layouts.data();

    descriptor_sets_.resize(swapchain_images_.size());
    if (vkAllocateDescriptorSets(device_, &alloc_info, descriptor_sets_.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < swapchain_images_.size(); i++) {
        VkDescriptorBufferInfo buffer_info{};
        buffer_info.buffer = uniform_buffers_[i];
        buffer_info.offset = 0;
        buffer_info.range = sizeof(UniformBufferObject);

        std::array<VkWriteDescriptorSet, 1> descriptor_writes{};

        descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[0].dstSet = descriptor_sets_[i];
        descriptor_writes[0].dstBinding = 0;
        descriptor_writes[0].dstArrayElement = 0;
        descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_writes[0].descriptorCount = 1;
        descriptor_writes[0].pBufferInfo = &buffer_info;

        vkUpdateDescriptorSets(device_, static_cast<uint32_t>(descriptor_writes.size()),
                               descriptor_writes.data(), 0, nullptr);
    }
}

void TriangleApplication::CreateCommandBuffers() {
    command_buffers_.resize(framebuffers_.size());
    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = command_pool_;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = (uint32_t) command_buffers_.size();

    if (vkAllocateCommandBuffers(device_, &alloc_info, command_buffers_.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }

    for (size_t i = 0; i < command_buffers_.size(); i++) {
        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(command_buffers_[i], &begin_info) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo render_pass_begin_info{};
        render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_begin_info.renderPass = render_pass_;
        render_pass_begin_info.framebuffer = framebuffers_[i];
        render_pass_begin_info.renderArea.offset = {0, 0};
        render_pass_begin_info.renderArea.extent = swapchain_extent_;

        std::array<VkClearValue, 2> clear_values{};
        clear_values[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
        clear_values[1].depthStencil = {1.0f, 0};

        render_pass_begin_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
        render_pass_begin_info.pClearValues = clear_values.data();

        vkCmdBeginRenderPass(command_buffers_[i], &render_pass_begin_info,
                             VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(command_buffers_[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline_);

        VkBuffer vertex_buffers[] = {vertex_buffer_};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(command_buffers_[i], 0, 1, vertex_buffers, offsets);
        vkCmdBindIndexBuffer(command_buffers_[i], index_buffer_, 0, VK_INDEX_TYPE_UINT16);
        vkCmdBindDescriptorSets(command_buffers_[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
                                pipeline_layout_, 0, 1, &descriptor_sets_[i], 0, nullptr);

        vkCmdDrawIndexed(command_buffers_[i], indices_.size(), 1, 0, 0, 0);

        vkCmdEndRenderPass(command_buffers_[i]);

        if (vkEndCommandBuffer(command_buffers_[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }
}