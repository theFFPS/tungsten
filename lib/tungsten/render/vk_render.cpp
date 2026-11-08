#include <tungsten/render/vk_renderer.hpp>
#include <tungsten/render/vk_asset_loader.hpp>
#include <cstring>

namespace tungsten {

vk_renderer::vk_renderer(SDL_Window* w, vk_context& c)
    : window(w), ctx(&c), swapchain(c.surface), descriptors(c.device, max_frames_in_flight)
{
    swapchain.create(window, c.device, c.physical_device);
    create_depth();

    sampler = c.device.createSampler(vk::SamplerCreateInfo{
        .magFilter        = vk::Filter::eLinear,
        .minFilter        = vk::Filter::eLinear,
        .mipmapMode       = vk::SamplerMipmapMode::eLinear,
        .addressModeU     = vk::SamplerAddressMode::eRepeat,
        .addressModeV     = vk::SamplerAddressMode::eRepeat,
        .addressModeW     = vk::SamplerAddressMode::eRepeat,
        .anisotropyEnable = vk::True,
        .maxAnisotropy    = c.physical_device.getProperties().limits.maxSamplerAnisotropy,
    });

    // ---- default pipeline ----
    vk_pipeline_config cfg{
        .name     = "default",
        .vert_spv = "/res/shader/basic.vert.spv",
        .frag_spv = "/res/shader/basic.frag.spv",
        .blend_enable = true,
    };
    auto vert = vfs_read_spirv(cfg.vert_spv);
    auto frag = vfs_read_spirv(cfg.frag_spv);
    default_pipeline = vk_pipeline(c.device, descriptors.set_layout,
                                   swapchain.format, vk::Format::eD32Sfloat,
                                   cfg, vert, frag);

    // ---- per-frame resources ----
    frames.resize(max_frames_in_flight);
    std::vector<vk::CommandBuffer> cmdbufs = c.device.allocateCommandBuffers(
        vk::CommandBufferAllocateInfo{
            .commandPool = c.command_pool,
            .commandBufferCount = max_frames_in_flight });

    for (uint32_t i = 0; i < max_frames_in_flight; ++i) {
        auto& f = frames[i];
        f.cmd           = std::move(cmdbufs[i]);
        f.image_ready   = c.device.createSemaphore({});
        f.render_done   = c.device.createSemaphore({});
        f.in_flight     = c.device.createFence({ .flags = vk::FenceCreateFlagBits::eSignaled });

        vk::BufferCreateInfo bci{
            .size = sizeof(vk_camera_ubo),
            .usage = vk::BufferUsageFlagBits::eUniformBuffer,
        };
        VmaAllocationCreateInfo vai{
            .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                     VMA_ALLOCATION_CREATE_MAPPED_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO,
        };
        VmaAllocationInfo ai{};
        VkBuffer raw;
        vmaCreateBuffer(c.allocator, reinterpret_cast<VkBufferCreateInfo*>(&bci), &vai, &raw, &f.camera_alloc, &ai);
        f.camera_buffer = vk::raii::Buffer(c.device, raw);
        f.camera_mapped = ai.pMappedData;
    }

    auto sets = descriptors.allocate(c.device, max_frames_in_flight);
    for (uint32_t i = 0; i < max_frames_in_flight; ++i) {
        frames[i].descriptor_set = std::move(sets[i]);
        descriptors.update_camera(c.device, frames[i].descriptor_set,
                                  frames[i].camera_buffer, sizeof(vk_camera_ubo));
    }
}

vk_renderer::~vk_renderer() {
    ctx->device.waitIdle();
    for (auto& f : frames) {
        if (f.camera_alloc) vmaDestroyBuffer(ctx->allocator, *f.camera_buffer, f.camera_alloc);
        f.camera_buffer = nullptr;
    }
    textures.clear();
    if (depth_alloc) vmaDestroyImage(ctx->allocator, *depth_image, depth_alloc);
}

void vk_renderer::create_depth() {
    if (depth_alloc) {
        depth_view  = nullptr;
        vmaDestroyImage(ctx->allocator, *depth_image, depth_alloc);
        depth_image = nullptr;
        depth_alloc = nullptr;
    }
    vk::ImageCreateInfo ici{
        .imageType = vk::ImageType::e2D,
        .format    = vk::Format::eD32Sfloat,
        .extent    = { swapchain.extent.width, swapchain.extent.height, 1 },
        .mipLevels = 1, .arrayLayers = 1,
        .samples   = vk::SampleCountFlagBits::e1,
        .tiling    = vk::ImageTiling::eOptimal,
        .usage     = vk::ImageUsageFlagBits::eDepthStencilAttachment,
        .sharingMode = vk::SharingMode::eExclusive,
    };
    VmaAllocationCreateInfo vai{ .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE };
    VkImage raw;
    vmaCreateImage(ctx->allocator, reinterpret_cast<VkImageCreateInfo*>(&ici), &vai, &raw, &depth_alloc, nullptr);
    depth_image = vk::raii::Image(ctx->device, raw);
    depth_view  = ctx->device.createImageView(vk::ImageViewCreateInfo{
        .image    = depth_image,
        .viewType = vk::ImageViewType::e2D,
        .format   = vk::Format::eD32Sfloat,
        .subresourceRange = { vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1 },
    });
}

uint32_t vk_renderer::add_texture(vk_texture&& tex) {
    uint32_t idx = static_cast<uint32_t>(textures.size());
    texture_infos.push_back(vk::DescriptorImageInfo{
        .sampler     = sampler,
        .imageView   = tex.view,
        .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
    });
    textures.push_back(std::move(tex));
    textures_dirty = true;
    return idx;
}

void vk_renderer::submit(vk_model&& model) {
    models.push_back(std::move(model));
}

void vk_renderer::clear_scene() {
    models.clear();
}

void vk_renderer::rebuild_descriptors() {
    for (auto& f : frames) {
        descriptors.update_textures(ctx->device, f.descriptor_set, texture_infos);
    }
    textures_dirty = false;
}

void vk_renderer::upload_camera(vk_frame& frame, const vk_camera& cam) {
    float aspect = static_cast<float>(swapchain.extent.width) /
                   static_cast<float>(swapchain.extent.height);
    auto ubo = cam.to_ubo(aspect);
    memcpy(frame.camera_mapped, &ubo, sizeof(ubo));
}

void vk_renderer::handle_resize() {
    ctx->device.waitIdle();
    if (swapchain.recreate(window, ctx->device, ctx->physical_device)) {
        create_depth();
        resized = false;
    }
}

void vk_renderer::draw_frame(const vk_camera& cam) {
    if (resized) handle_resize();
    if (textures_dirty) rebuild_descriptors();

    auto& frame = frames[current_frame];

    // wait for previous frame using this slot
    if (ctx->device.waitForFences(*frame.in_flight, true, UINT64_MAX) != vk::Result::eSuccess)
        throw std::runtime_error("waitForFences failed");

    uint32_t image_idx;
    {
        auto [result, idx] = swapchain.acquire_next(frame.image_ready);
        if (result == vk::Result::eErrorOutOfDateKHR) { resized = true; return; }
        if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
            throw std::runtime_error("acquireNextImage failed");
        image_idx = idx;
    }

    ctx->device.resetFences(*frame.in_flight);
    upload_camera(frame, cam);
    frame.cmd.reset();
    frame.cmd.begin(vk::CommandBufferBeginInfo{ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit });

    // ---- layout transitions ----
    std::array<vk::ImageMemoryBarrier2, 2> barriers{
        vk::ImageMemoryBarrier2{
            .srcStageMask  = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
            .srcAccessMask = {},
            .dstStageMask  = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
            .dstAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite,
            .oldLayout     = vk::ImageLayout::eUndefined,
            .newLayout     = vk::ImageLayout::eColorAttachmentOptimal,
            .image         = swapchain.images[image_idx],
            .subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 },
        },
        vk::ImageMemoryBarrier2{
            .srcStageMask  = vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
            .srcAccessMask = vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
            .dstStageMask  = vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
            .dstAccessMask = vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
            .oldLayout     = vk::ImageLayout::eUndefined,
            .newLayout     = vk::ImageLayout::eDepthAttachmentOptimal,
            .image         = depth_image,
            .subresourceRange = { vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1 },
        },
    };
    frame.cmd.pipelineBarrier2(vk::DependencyInfo{
        .imageMemoryBarrierCount = static_cast<uint32_t>(barriers.size()),
        .pImageMemoryBarriers    = barriers.data(),
    });

    // ---- rendering ----
    vk::RenderingAttachmentInfo color_att{
        .imageView   = swapchain.image_views[image_idx],
        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .loadOp      = vk::AttachmentLoadOp::eClear,
        .storeOp     = vk::AttachmentStoreOp::eStore,
        .clearValue  = vk::ClearValue{ vk::ClearColorValue{ std::array{ 0.05f, 0.05f, 0.08f, 1.0f } } },
    };
    vk::RenderingAttachmentInfo depth_att{
        .imageView   = depth_view,
        .imageLayout = vk::ImageLayout::eDepthAttachmentOptimal,
        .loadOp      = vk::AttachmentLoadOp::eClear,
        .storeOp     = vk::AttachmentStoreOp::eDontCare,
        .clearValue  = vk::ClearValue{ vk::ClearDepthStencilValue{ 1.0f, 0 } },
    };
    vk::RenderingInfo rendering_info{
        .renderArea           = { { 0, 0 }, swapchain.extent },
        .layerCount           = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments    = &color_att,
        .pDepthAttachment     = &depth_att,
    };
    frame.cmd.beginRendering(rendering_info);
    frame.cmd.setViewport(0, vk::Viewport{
        0.0f, 0.0f,
        static_cast<float>(swapchain.extent.width),
        static_cast<float>(swapchain.extent.height),
        0.0f, 1.0f });
    frame.cmd.setScissor(0, vk::Rect2D{ {0, 0}, swapchain.extent });

    default_pipeline.bind(frame.cmd);
    frame.cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                 default_pipeline.layout, 0,
                                 *frame.descriptor_set, nullptr);

    for (const auto& model : models) {
        for (const auto& mesh : model.meshes) {
            vk_push_constants pc{
                .model         = model.transform,
                .texture_index = mesh.texture_index,
            };
            frame.cmd.pushConstants(*default_pipeline.layout,
                vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
                0, sizeof(pc), &pc);
            mesh.draw(frame.cmd);
        }
    }

    frame.cmd.endRendering();

    // ---- present layout ----
    vk::ImageMemoryBarrier2 present_barrier{
        .srcStageMask  = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        .srcAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite,
        .dstStageMask  = vk::PipelineStageFlagBits2::eBottomOfPipe,
        .oldLayout     = vk::ImageLayout::eColorAttachmentOptimal,
        .newLayout     = vk::ImageLayout::ePresentSrcKHR,
        .image         = swapchain.images[image_idx],
        .subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 },
    };
    frame.cmd.pipelineBarrier2(vk::DependencyInfo{
        .imageMemoryBarrierCount = 1, .pImageMemoryBarriers = &present_barrier });

    frame.cmd.end();

    vk::PipelineStageFlags wait_stage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    ctx->graphics_queue.submit(vk::SubmitInfo{
        .waitSemaphoreCount   = 1,
        .pWaitSemaphores      = &*frame.image_ready,
        .pWaitDstStageMask    = &wait_stage,
        .commandBufferCount   = 1,
        .pCommandBuffers      = &*frame.cmd,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores    = &*frame.render_done,
    }, frame.in_flight);

    auto present_result = swapchain.present(ctx->graphics_queue, image_idx, frame.render_done);
    if (present_result == vk::Result::eSuboptimalKHR || present_result == vk::Result::eErrorOutOfDateKHR)
        resized = true;

    current_frame = (current_frame + 1) % max_frames_in_flight;
}

} // namespace tungsten
