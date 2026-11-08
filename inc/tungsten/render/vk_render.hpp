#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <tungsten/render/vk_context.hpp>
#include <tungsten/render/vk_swapchain.hpp>
#include <tungsten/render/vk_descriptors.hpp>
#include <tungsten/render/vk_texture.hpp>
#include <tungsten/render/vk_pipeline.hpp>
#include <tungsten/render/vk_model.hpp>
#include <tungsten/render/vk_camera.hpp>
#include <SDL3/SDL.h>

namespace tungsten {

struct vk_frame {
    vk::raii::CommandBuffer cmd           = nullptr;
    vk::raii::Semaphore     image_ready   = nullptr;
    vk::raii::Semaphore     render_done   = nullptr;
    vk::raii::Fence         in_flight     = nullptr;
    vk::raii::Buffer        camera_buffer = nullptr;
    VmaAllocation           camera_alloc  = nullptr;
    void*                   camera_mapped = nullptr;
    vk::raii::DescriptorSet descriptor_set = nullptr;
};

struct vk_renderer {
    static constexpr uint32_t max_frames_in_flight = 2;

    SDL_Window*      window    = nullptr;
    vk_context*      ctx       = nullptr;

    vk_swapchain     swapchain;
    vk_descriptors   descriptors;
    vk_pipeline      default_pipeline;
    vk::raii::Sampler sampler = nullptr;

    std::vector<vk_frame>        frames;
    std::vector<vk_texture>      textures;
    std::vector<vk::DescriptorImageInfo> texture_infos;
    std::vector<vk_model>        models;

    // depth
    vk::raii::Image        depth_image = nullptr;
    VmaAllocation          depth_alloc = nullptr;
    vk::raii::ImageView    depth_view  = nullptr;

    uint32_t current_frame = 0;
    bool     resized       = false;
    bool     textures_dirty = false;

    vk_renderer(SDL_Window* window, vk_context& ctx);
    ~vk_renderer();

    vk_renderer(const vk_renderer&) = delete;
    vk_renderer(vk_renderer&&)      = delete;

    // API for game code
    uint32_t  add_texture(vk_texture&& tex);              // returns bindless index
    void      submit(vk_model&& model);
    void      clear_scene();
    void      draw_frame(const vk_camera& cam);

    void      handle_resize();

private:
    void create_depth();
    void rebuild_descriptors();
    void upload_camera(vk_frame& frame, const vk_camera& cam);
};

} // namespace tungsten
