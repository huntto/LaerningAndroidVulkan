#include "model_application.h"

#include <array>
#include <istream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <glm/gtc/matrix_transform.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <log.h>
#include <filesystem.h>

ModelApplication::ModelApplication(void *native_window, std::vector<char> vert_shader_code,
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

void ModelApplication::Init() {
    CreateModel();
    VulkanApplication::Init();
}

void ModelApplication::Draw() {
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

    UpdateUniformBuffer(image_index);

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

void ModelApplication::Cleanup() {
    vkDestroySampler(device_, texture_sampler_, nullptr);
    vkDestroyImageView(device_, texture_image_view_, nullptr);
    vkDestroyImage(device_, texture_image_, nullptr);
    vkFreeMemory(device_, texture_image_memory_, nullptr);
    VulkanApplication::Cleanup();
}

void ModelApplication::Rotate(float radius, float x, float y, float z) {
    glm::mat4 tmp = glm::mat4(ubo_.model);
    ubo_.model = glm::rotate(glm::mat4(1.0f), radius, glm::vec3(x, y, z)) * tmp;
}

void ModelApplication::CreateDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding ubo_layout_binding{};
    ubo_layout_binding.binding = 0;
    ubo_layout_binding.descriptorCount = 1;
    ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    ubo_layout_binding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding sampler_layout_binding{};
    sampler_layout_binding.binding = 1;
    sampler_layout_binding.descriptorCount = 1;
    sampler_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    sampler_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    sampler_layout_binding.pImmutableSamplers = nullptr;

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = {ubo_layout_binding,
                                                            sampler_layout_binding};

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

void ModelApplication::CreateTextureImage() {
    int tex_width, tex_height, tex_channels;
    auto img = tiny_engine::Filesystem::GetInstance().Read<stbi_uc>("textures/viking_room.png");
    stbi_uc *pixels = stbi_load_from_memory(img.data(),
                                            img.size(),
                                            &tex_width,
                                            &tex_height,
                                            &tex_channels,
                                            STBI_rgb_alpha);
    VkDeviceSize image_size = tex_width * tex_height * 4;

    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }

    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    CreateBuffer(physical_device_,
                 device_,
                 image_size,
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 staging_buffer,
                 staging_buffer_memory);

    void *data;
    vkMapMemory(device_, staging_buffer_memory, 0, image_size, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(image_size));
    vkUnmapMemory(device_, staging_buffer_memory);

    stbi_image_free(pixels);

    CreateImage(physical_device_,
                device_,
                tex_width,
                tex_height,
                VK_FORMAT_R8G8B8A8_SRGB,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                texture_image_,
                texture_image_memory_);

    TransitionImageLayout(device_,
                          command_pool_,
                          graphics_queue_,
                          texture_image_,
                          VK_FORMAT_R8G8B8A8_SRGB,
                          VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    CopyBufferToImage(device_,
                      command_pool_,
                      graphics_queue_,
                      staging_buffer,
                      texture_image_,
                      static_cast<uint32_t>(tex_width),
                      static_cast<uint32_t>(tex_height));

    TransitionImageLayout(device_,
                          command_pool_,
                          graphics_queue_,
                          texture_image_,
                          VK_FORMAT_R8G8B8A8_SRGB,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(device_, staging_buffer, nullptr);
    vkFreeMemory(device_, staging_buffer_memory, nullptr);
}

void ModelApplication::CreateTextureImageView() {
    texture_image_view_ = CreateImageView(device_,
                                          texture_image_,
                                          VK_FORMAT_R8G8B8A8_SRGB,
                                          VK_IMAGE_ASPECT_COLOR_BIT);
}

void ModelApplication::CreateTextureSampler() {
    VkSamplerCreateInfo sampler_create_info{};
    sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_create_info.magFilter = VK_FILTER_LINEAR;
    sampler_create_info.minFilter = VK_FILTER_LINEAR;
    sampler_create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_create_info.anisotropyEnable = VK_FALSE;
    sampler_create_info.maxAnisotropy = 16.0f;
    sampler_create_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    sampler_create_info.unnormalizedCoordinates = VK_FALSE;
    sampler_create_info.compareEnable = VK_FALSE;
    sampler_create_info.compareOp = VK_COMPARE_OP_ALWAYS;
    sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    if (vkCreateSampler(device_, &sampler_create_info, nullptr, &texture_sampler_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
}

void ModelApplication::CreateVertexBuffer() {
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

void ModelApplication::CreateIndexBuffer() {
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

void ModelApplication::CreateUniformBuffers() {
    VkDeviceSize buffer_size = sizeof(UniformBufferObject);

    uniform_buffers_.resize(swapchain_images_.size());
    uniform_buffers_memory_.resize(swapchain_images_.size());

    ubo_.model = glm::mat4(1.0f);
    ubo_.view = glm::lookAt(glm::vec3(0.0f, 0.0f, -6.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                            glm::vec3(0.0f, 1.0f, 0.0f));
    ubo_.proj = glm::perspective(glm::radians(45.0f),
                                 swapchain_extent_.width / (float) swapchain_extent_.height, 0.1f,
                                 10.0f);
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

void ModelApplication::CreateDescriptorPool() {
    std::array<VkDescriptorPoolSize, 2> pool_sizes{};
    pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pool_sizes[0].descriptorCount = static_cast<uint32_t>(swapchain_images_.size());
    pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pool_sizes[1].descriptorCount = static_cast<uint32_t>(swapchain_images_.size());

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

void ModelApplication::CreateDescriptorSets() {
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

        VkDescriptorImageInfo image_info{};
        image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        image_info.imageView = texture_image_view_;
        image_info.sampler = texture_sampler_;

        std::array<VkWriteDescriptorSet, 2> descriptor_writes{};

        descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[0].dstSet = descriptor_sets_[i];
        descriptor_writes[0].dstBinding = 0;
        descriptor_writes[0].dstArrayElement = 0;
        descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_writes[0].descriptorCount = 1;
        descriptor_writes[0].pBufferInfo = &buffer_info;

        descriptor_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[1].dstSet = descriptor_sets_[i];
        descriptor_writes[1].dstBinding = 1;
        descriptor_writes[1].dstArrayElement = 0;
        descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptor_writes[1].descriptorCount = 1;
        descriptor_writes[1].pImageInfo = &image_info;

        vkUpdateDescriptorSets(device_, static_cast<uint32_t>(descriptor_writes.size()),
                               descriptor_writes.data(), 0, nullptr);
    }
}

void ModelApplication::CreateCommandBuffers() {
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

void ModelApplication::UpdateUniformBuffer(uint32_t current_image) {
    void *data;
    vkMapMemory(device_, uniform_buffers_memory_[current_image], 0, sizeof(ubo_), 0, &data);
    memcpy(data, &ubo_, sizeof(ubo_));
    vkUnmapMemory(device_, uniform_buffers_memory_[current_image]);
}

void ModelApplication::CreateModel() {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    std::string buffer;
    tiny_engine::Filesystem::GetInstance().Read("models/viking_room.obj", buffer);

    std::stringstream sstream(buffer);
    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, &sstream)) {
        throw std::runtime_error(warn + err);
    }

    std::unordered_map<Vertex, uint32_t> unique_vertices{};

    for (const auto &shape : shapes) {
        for (const auto &index : shape.mesh.indices) {
            Vertex vertex{};

            vertex.pos = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
            };

            vertex.tex_coord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
            };

            vertex.color = {1.0f, 1.0f, 1.0f};

            if (unique_vertices.count(vertex) == 0) {
                unique_vertices[vertex] = static_cast<uint32_t>(vertices_.size());
                vertices_.push_back(vertex);
            }

            indices_.push_back(unique_vertices[vertex]);
        }
    }
}