#include "Renderer.h"

UniformBufferObject ubo{
        .view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f),
                            glm::vec3(0.0f, 0.0f, 0.0f),
                            glm::vec3(0.0f, 0.0f, 1.0f)),

        .proj = glm::perspective(glm::radians(V_FOV), 1600.0f / 900.0f, 0.1f, 10.0f),
};

void Renderer::registerDrawGroup(DrawGroup&& drawGroup) {
    drawGroups.push_back(drawGroup);
    drawGroups.back().drawGroupIndex = drawGroups.size() - 1;
}

void Renderer::upload_uniform_buffer_task(daxa::TaskGraph& tg, const daxa::TaskBufferView uniform_buffer, const UniformBufferObject &ubo) {
    tg.add_task({
        .attachments = {
            daxa::inl_attachment(daxa::TaskBufferAccess::TRANSFER_WRITE, uniform_buffer)
        },
        .task = [=](const daxa::TaskInterface &ti) {
            auto uniform_staging = ti.device.create_buffer({
                           .size = sizeof(UniformBufferObject),
                           .allocate_info = daxa::MemoryFlagBits::HOST_ACCESS_RANDOM,
                           .name = "uniform staging buffer",
            });
            ti.recorder.destroy_buffer_deferred(uniform_staging);
            auto* uniform_ptr = ti.device.buffer_host_address_as<UniformBufferObject>(uniform_staging).value();
            *uniform_ptr = ubo;
            ti.recorder.copy_buffer_to_buffer({
                .src_buffer = uniform_staging,
                .dst_buffer = ti.get(uniform_buffer).ids[0],
                .size = sizeof(UniformBufferObject),
            });
        },
        .name = "upload uniform data",
        });
}

void Renderer::draw_mesh_task(
    const DrawGroup& drawGroup,
    const bool clear
) {
    loop_task_graph.use_persistent_buffer(drawGroup.task_vertex_buffer);
    loop_task_graph.use_persistent_buffer(drawGroup.task_index_buffer);
    loop_task_graph.use_persistent_buffer(drawGroup.task_instance_buffer);

    std::vector<daxa::TaskAttachmentInfo> attachments;

    // Shared resources
    attachments.push_back(daxa::inl_attachment(daxa::TaskBufferAccess::VERTEX_SHADER_READ, task_uniform_buffer));
    attachments.push_back(daxa::inl_attachment(daxa::TaskImageAccess::COLOR_ATTACHMENT, daxa::ImageViewType::REGULAR_2D, task_swapchain_image));
    attachments.push_back(daxa::inl_attachment(daxa::TaskImageAccess::DEPTH_ATTACHMENT, daxa::ImageViewType::REGULAR_2D, task_z_buffer));

    // Add each drawable's vertex/index/instance buffers
    attachments.push_back(daxa::inl_attachment(daxa::TaskBufferAccess::VERTEX_SHADER_READ, drawGroup.task_vertex_buffer));
    attachments.push_back(daxa::inl_attachment(daxa::TaskBufferAccess::VERTEX_SHADER_READ, drawGroup.task_instance_buffer));
    attachments.push_back(daxa::inl_attachment(daxa::TaskBufferAccess::INDEX_READ, drawGroup.task_index_buffer));

    loop_task_graph.add_task({
        .attachments = attachments,
        .task = [=](const daxa::TaskInterface& ti) {
            auto const size = ti.device.info(ti.get(task_swapchain_image).ids[0]).value().size;
            daxa::RenderCommandRecorder render_recorder = std::move(ti.recorder).begin_renderpass({
                .color_attachments = std::array{
                    daxa::RenderAttachmentInfo{
                        .image_view = ti.get(task_swapchain_image).view_ids[0],
                        .load_op = clear ? daxa::AttachmentLoadOp::CLEAR : daxa::AttachmentLoadOp::LOAD,
                        .clear_value = std::array<daxa::f32, 4>{0.1f, 0.0f, 0.5f, 1.0f},
                    },
                },
                .depth_attachment = daxa::RenderAttachmentInfo{
                    .image_view = ti.get(task_z_buffer).view_ids[0],
                    .load_op = clear ? daxa::AttachmentLoadOp::CLEAR : daxa::AttachmentLoadOp::LOAD,
                    .clear_value = daxa::DepthValue{1.0f, 0},
                },
                .render_area = {.width = size.x, .height = size.y},
            });

            render_recorder.set_pipeline(*drawGroup.pipeline);

            for (auto const& drawableMesh : drawGroup.meshes) {
                render_recorder.set_index_buffer({
                    .id = ti.get(drawGroup.task_index_buffer).ids[0],
                    .offset = 0,
                    .index_type = daxa::IndexType::uint32,
                });

                render_recorder.push_constant(PushConstant{
                    .vertex_ptr = ti.device.device_address(ti.get(drawGroup.task_vertex_buffer).ids[0]).value(),
                    .ubo_ptr = ti.device.device_address(ti.get(task_uniform_buffer).ids[0]).value(),
                    .instance_buffer_ptr = ti.device.device_address(ti.get(drawGroup.task_instance_buffer).ids[0]).value(),
                });

                render_recorder.draw_indexed({
                    .index_count = drawableMesh.lock()->index_count,
                    .instance_count = static_cast<std::uint32_t>(drawableMesh.lock()->instance_data.size()),
                    .first_index = drawableMesh.lock()->index_offset,
                    .vertex_offset = static_cast<std::int32_t>(drawableMesh.lock()->vertex_offset),
                    .first_instance = drawableMesh.lock()->instance_offset
                });
            }
            ti.recorder = std::move(render_recorder).end_renderpass();
        },
        .name = "draw mesh",
        });
}

void Renderer::update_uniform_buffer(const daxa::Device& device, const daxa::BufferId uniform_buffer_id, Camera camera, float aspect_ratio) {
    UniformBufferObject ubo{};
    ubo.view = camera.get_view_matrix();
    ubo.proj = camera.get_projection(aspect_ratio);

    auto* ptr = device.buffer_host_address_as<UniformBufferObject>(uniform_buffer_id).value();
    *ptr = ubo;
}

Renderer::Renderer(GLFW_Window::AppWindow& window, daxa::Device& device, daxa::Instance& instance)
    : window(window), device(device), instance(instance) {

    swapchain = device.create_swapchain({
        .native_window = window.get_native_handle(),
        .native_window_platform = GLFW_Window::AppWindow::get_native_platform(),
        .present_mode = daxa::PresentMode::FIFO,
        .image_usage = daxa::ImageUsageFlagBits::TRANSFER_DST,
        .name = "swapchain",
    });
}

void Renderer::init() {
    auto size = swapchain.get_surface_extent();

    pipeline_manager = daxa::PipelineManager({
        .device = device,
        .root_paths = {
            DAXA_SHADER_INCLUDE_DIR,
            GLOBAL_SHADER_PATH,
        },
        .default_language = std::optional{daxa::ShaderLanguage::GLSL},
        .default_enable_debug_info = std::optional{true},
        .name = "pipeline manager",
    });

    uniform_buffer_id = device.create_buffer({
        .size = sizeof(UniformBufferObject),
        .allocate_info = daxa::MemoryFlagBits::HOST_ACCESS_RANDOM,
        .name = "Uniform buffer MVP",
    });

    task_uniform_buffer = daxa::TaskBuffer({
        .initial_buffers = {.buffers = std::span{&uniform_buffer_id, 1}},
        .name = "task uniform buffer",
    });
    // Create the z-buffer
    z_buffer_id = device.create_image({
        .format = daxa::Format::D32_SFLOAT,
        .size = {size.x, size.y, 1},
        .usage = daxa::ImageUsageFlagBits::DEPTH_STENCIL_ATTACHMENT,
        .name = "z-buffer",
    });

    task_z_buffer = daxa::TaskImage({
        .initial_images = {.images = std::span{&z_buffer_id, 1}},
        .name = "task depth image",
    });

    task_swapchain_image = daxa::TaskImage{ {.swapchain_image = true, .name = "swapchain image"} };

    loop_task_graph.use_persistent_buffer(task_uniform_buffer);
    loop_task_graph.use_persistent_image(task_z_buffer);
    loop_task_graph.use_persistent_image(task_swapchain_image);
}

void Renderer::submit_task_graph() {
    bool firstDrawMeshTask = true;
    // TODO: abstractify the task addition
    for (auto& drawGroup : drawGroups) {
        draw_mesh_task(drawGroup, firstDrawMeshTask);
        firstDrawMeshTask = false;
    }

    loop_task_graph.submit({});
    // And tell the task graph to do the present step.
    loop_task_graph.present({});
    // Finally, we complete the task graph, which essentially compiles the
    // dependency graph between tasks, and inserts the most optimal synchronization!
    loop_task_graph.complete({});
}

void Renderer::cleanup() {
    for (auto& drawGroup : drawGroups) {
        drawGroup.cleanup();
    }
    device.destroy_image(z_buffer_id);
    device.destroy_buffer(uniform_buffer_id);
}

void Renderer::startFrame(const Camera& camera) {
    if (window.swapchain_out_of_date) {
        swapchain.resize();
        window.swapchain_out_of_date = false;

        // Recreate our buffers
        device.destroy_image(z_buffer_id);
        auto size = swapchain.get_surface_extent();
        z_buffer_id = device.create_image({
            .format = daxa::Format::D32_SFLOAT,
            .size = {size.x, size.y, 1},
            .usage = daxa::ImageUsageFlagBits::DEPTH_STENCIL_ATTACHMENT,
            .name = "z-buffer",
        });
        task_z_buffer.set_images({ .images = std::span{&z_buffer_id, 1} });
    }

    float aspect_ratio = static_cast<float>(window.width) / static_cast<float>(window.height);
    update_uniform_buffer(device, uniform_buffer_id, camera, aspect_ratio);
}

void Renderer::endFrame() {
    auto swapchain_image = swapchain.acquire_next_image();

    task_swapchain_image.set_images({ .images = std::span{&swapchain_image, 1} });

    loop_task_graph.execute({});
    device.collect_garbage();
}
