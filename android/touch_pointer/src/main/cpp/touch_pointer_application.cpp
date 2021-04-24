#include "touch_pointer_application.h"

#include <array>
#include <glm/gtc/matrix_transform.hpp>
#include <cstdlib>
#define LOG_TAG "TouchPointer"
#include <log.h>

TouchPointerApplication::TouchPointerApplication(void *native_window,
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
    application_name_ = "TouchPointer";
    native_window_ = native_window;
    vert_shader_code_ = vert_shader_code;
    frag_shader_code_ = frag_shader_code;
    binding_descriptions_ = Vertex::GetBindingDescription();
    attribute_descriptions_ = Vertex::GetAttributeDescriptions();
    max_frames_in_flight_ = 2;
    primitive_topology_ = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
}

void TouchPointerApplication::Draw() {
    vkWaitForFences(device_, 1, &in_flight_fences_[current_frame_], VK_TRUE, UINT64_MAX);

    uint32_t image_index;
    VkResult result = vkAcquireNextImageKHR(device_, swapchain_, UINT64_MAX,
                                            image_available_semaphores_[current_frame_],
                                            VK_NULL_HANDLE,
                                            &image_index);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        LOGI("RecreateSwapChain cause of VK_ERROR_OUT_OF_DATE_KHR");
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    Update(current_frame_);

    if (images_in_flight_[image_index] != VK_NULL_HANDLE) {
        vkWaitForFences(device_, 1, &images_in_flight_[image_index], VK_TRUE, UINT64_MAX);
    }
    images_in_flight_[image_index] = in_flight_fences_[current_frame_];

    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore wait_semaphores[] = {image_available_semaphores_[current_frame_]};
    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.pWaitDstStageMask = wait_stages;

    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffers_[image_index];

    VkSemaphore signal_semaphores[] = {render_finished_semaphores_[current_frame_]};
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores;

    vkResetFences(device_, 1, &in_flight_fences_[current_frame_]);

    if (vkQueueSubmit(graphics_queue_, 1, &submit_info, in_flight_fences_[current_frame_]) !=
        VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signal_semaphores;

    VkSwapchainKHR swapchains[] = {swapchain_};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;

    presentInfo.pImageIndices = &image_index;

    result = vkQueuePresentKHR(present_queue_, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        LOGI("RecreateSwapChain cause of result:%d", result);
        return;
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    current_frame_ = (current_frame_ + 1) % max_frames_in_flight_;
    vkDeviceWaitIdle(device_);
}

void TouchPointerApplication::CreateDescriptorSetLayout() {
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

void TouchPointerApplication::CreateVertexBuffer() {
    VkBufferCreateInfo buffer_create_info{};
    buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_create_info.size = sizeof(vertices_[0]) * vertices_.size();
    buffer_create_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device_, &buffer_create_info, nullptr, &vertex_buffer_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create vertex buffer!");
    }

    VkMemoryRequirements mem_requirements;
    vkGetBufferMemoryRequirements(device_, vertex_buffer_, &mem_requirements);

    VkMemoryAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_requirements.size;
    alloc_info.memoryTypeIndex = FindMemoryType(physical_device_,
                                                mem_requirements.memoryTypeBits,
                                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    if (vkAllocateMemory(device_, &alloc_info, nullptr, &vertex_buffer_memory_) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate vertex buffer memory!");
    }
    vkBindBufferMemory(device_, vertex_buffer_, vertex_buffer_memory_, 0);

    void *data;
    vkMapMemory(device_, vertex_buffer_memory_, 0, buffer_create_info.size, 0, &data);
    memcpy(data, vertices_.data(), (size_t) buffer_create_info.size);
    vkUnmapMemory(device_, vertex_buffer_memory_);
}

void TouchPointerApplication::CreateUniformBuffers() {
    VkDeviceSize buffer_size = sizeof(UniformBufferObject);

    uniform_buffers_.resize(swapchain_images_.size());
    uniform_buffers_memory_.resize(swapchain_images_.size());

    float aspect_ratio = swapchain_extent_.width > swapchain_extent_.height
                         ? (float) swapchain_extent_.width / swapchain_extent_.height
                         : (float) swapchain_extent_.height / swapchain_extent_.width;

    ubo_.model = glm::mat4(1.0f);
    ubo_.view = glm::mat4(1.0f);
    ubo_.proj = glm::ortho(-1.0f, 1.0f,
                           -aspect_ratio, aspect_ratio,
                           -1.0f, 1.0f);
    ubo_.proj[1][1] *= -1;

    for (size_t i = 0; i < swapchain_images_.size(); i++) {
        CreateBuffer(physical_device_,
                     device_,
                     buffer_size,
                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     uniform_buffers_[i],
                     uniform_buffers_memory_[i]);


        void *data;
        vkMapMemory(device_, uniform_buffers_memory_[i], 0, sizeof(ubo_), 0, &data);
        memcpy(data, &ubo_, sizeof(ubo_));
        vkUnmapMemory(device_, uniform_buffers_memory_[i]);
    }
}

void TouchPointerApplication::CreateDescriptorPool() {
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

void TouchPointerApplication::CreateDescriptorSets() {
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

void TouchPointerApplication::CreateCommandBuffers() {
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
        clear_values[0].color = {1.0f, 1.0f, 1.0f, 1.0f};
        clear_values[1].depthStencil = {1.0f, 0};

        render_pass_begin_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
        render_pass_begin_info.pClearValues = clear_values.data();

        vkCmdBeginRenderPass(command_buffers_[i], &render_pass_begin_info,
                             VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(command_buffers_[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline_);

        VkBuffer vertex_buffers[] = {vertex_buffer_};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(command_buffers_[i], 0, 1, vertex_buffers, offsets);
        vkCmdBindDescriptorSets(command_buffers_[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
                                pipeline_layout_, 0, 1, &descriptor_sets_[i], 0, nullptr);

        vkCmdDraw(command_buffers_[i], vertices_.size(), 1, 0, 0);

        vkCmdEndRenderPass(command_buffers_[i]);

        if (vkEndCommandBuffer(command_buffers_[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }
}

static float RandomColor() {
    return (rand() % 100) / 100.0f;
}

void
TouchPointerApplication::UpdatePointer(int index, float x, float y, float size, Action action) {
    if (action == UP) {
        vertices_[index].pos.z = 100.0f;
        return;
    }

    vertices_[index].pos.x = x;
    vertices_[index].pos.y = y;
    vertices_[index].size = size;
    if (action == DOWN) {
        vertices_[index].color.r = RandomColor();
        vertices_[index].color.g = RandomColor();
        vertices_[index].color.b = RandomColor();
        vertices_[index].pos.x = x;
        vertices_[index].pos.y = y;
        vertices_[index].pos.z = 0.0f;
    }
}

void TouchPointerApplication::Update(uint32_t current_image) {
    VkDeviceSize vertex_size = sizeof(vertices_[0]) * vertices_.size();
    void *data;
    vkMapMemory(device_, vertex_buffer_memory_, 0, vertex_size, 0, &data);
    memcpy(data, vertices_.data(), (size_t) vertex_size);
    vkUnmapMemory(device_, vertex_buffer_memory_);
}
