#include "window.h"
#include "shared.inl"

#include <daxa/daxa.hpp>
#include <daxa/utils/pipeline_manager.hpp>
#include <daxa/utils/task_graph.hpp>

#include <iostream>

void upload_mesh_data_task(daxa::TaskGraph& tg, daxa::TaskBufferView vertices, daxa::TaskBufferView indices) {
    tg.add_task({
        .attachments = {
            daxa::inl_attachment(daxa::TaskBufferAccess::TRANSFER_WRITE, vertices),
            daxa::inl_attachment(daxa::TaskBufferAccess::TRANSFER_WRITE, indices),
        },
        .task = [=](daxa::TaskInterface ti) {
            auto vertex_data = std::array<MyVertex, 8>{
                MyVertex{.position = {-0.5f, -0.5f, -0.5f}, .color = {1, 0, 0}},
                MyVertex{.position = {+0.5f, -0.5f, -0.5f}, .color = {0, 1, 0}},
                MyVertex{.position = {+0.5f, +0.5f, -0.5f}, .color = {0, 0, 1}},
                MyVertex{.position = {-0.5f, +0.5f, -0.5f}, .color = {1, 1, 0}},
                MyVertex{.position = {-0.5f, -0.5f, +0.5f}, .color = {1, 0, 1}},
                MyVertex{.position = {+0.5f, -0.5f, +0.5f}, .color = {0, 1, 1}},
                MyVertex{.position = {+0.5f, +0.5f, +0.5f}, .color = {1, 1, 1}},
                MyVertex{.position = {-0.5f, +0.5f, +0.5f}, .color = {0, 0, 0}}
            };
            auto index_data = std::array<uint32_t, 36>{ 
                // front face
                0, 2, 1,  2, 0, 3,
                // right face
                1, 6, 5,  6, 1, 2,
                // back face
                5, 7, 4,  7, 5, 6,
                // left face
                4, 3, 0,  3, 4, 7,
                // top face
                3, 6, 2,  6, 3, 7,
                // bottom face
                4, 1, 5,  1, 4, 0,
            };

            auto vertex_staging = ti.device.create_buffer({
                           .size = sizeof(vertex_data),
                           .allocate_info = daxa::MemoryFlagBits::HOST_ACCESS_RANDOM,
                           .name = "vertex staging buffer",
            });
            ti.recorder.destroy_buffer_deferred(vertex_staging);
            auto* vertex_ptr = ti.device.buffer_host_address_as<std::array<MyVertex, 8>>(vertex_staging).value();
            *vertex_ptr = vertex_data;
            ti.recorder.copy_buffer_to_buffer({
                .src_buffer = vertex_staging,
                .dst_buffer = ti.get(vertices).ids[0],
                .size = sizeof(vertex_data),
            });

            auto index_staging = ti.device.create_buffer({
                .size = sizeof(index_data),
                .allocate_info = daxa::MemoryFlagBits::HOST_ACCESS_RANDOM,
                .name = "index staging buffer",
                });
            ti.recorder.destroy_buffer_deferred(index_staging);
            auto* index_ptr = ti.device.buffer_host_address_as<std::array<uint32_t, 36>>(index_staging).value();
            *index_ptr = index_data;
            ti.recorder.copy_buffer_to_buffer({
                .src_buffer = index_staging,
                .dst_buffer = ti.get(indices).ids[0],
                .size = sizeof(index_data),
            });
        },
        .name = "upload mesh data",
    });
}

void upload_uniform_buffer_task(daxa::TaskGraph& tg, daxa::TaskBufferView uniform_buffer, UniformBufferObject ubo) {
    tg.add_task({
        .attachments = {
            daxa::inl_attachment(daxa::TaskBufferAccess::TRANSFER_WRITE, uniform_buffer)
        },
        .task = [=](daxa::TaskInterface ti) {
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

void draw_mesh_task(
    daxa::TaskGraph& tg,
    std::shared_ptr<daxa::RasterPipeline> pipeline,
    daxa::TaskBufferView vertices,
    daxa::TaskBufferView indices,
    daxa::TaskImageView z_buffer,
    daxa::TaskBufferView uniform_buffer,
    daxa::TaskImageView render_target) {
    tg.add_task({
        .attachments = {
            daxa::inl_attachment(daxa::TaskBufferAccess::VERTEX_SHADER_READ, vertices),
            daxa::inl_attachment(daxa::TaskBufferAccess::VERTEX_SHADER_READ, uniform_buffer),
            daxa::inl_attachment(daxa::TaskBufferAccess::INDEX_READ, indices),
            daxa::inl_attachment(daxa::TaskImageAccess::COLOR_ATTACHMENT, daxa::ImageViewType::REGULAR_2D, render_target),
            daxa::inl_attachment(daxa::TaskImageAccess::DEPTH_ATTACHMENT, daxa::ImageViewType::REGULAR_2D, z_buffer),
        },
        .task = [=](daxa::TaskInterface ti) {
            auto const size = ti.device.info(ti.get(render_target).ids[0]).value().size;

            daxa::RenderCommandRecorder render_recorder = std::move(ti.recorder).begin_renderpass({
                .color_attachments = std::array{
                    daxa::RenderAttachmentInfo{
                        .image_view = ti.get(render_target).view_ids[0],
                        .load_op = daxa::AttachmentLoadOp::CLEAR,
                        .clear_value = std::array<daxa::f32, 4>{0.1f, 0.0f, 0.5f, 1.0f},
                    },
                },
                .depth_attachment = daxa::RenderAttachmentInfo{
                    .image_view = ti.get(z_buffer).view_ids[0],
                    .load_op = daxa::AttachmentLoadOp::CLEAR,
                    .clear_value = daxa::DepthValue{1.0f, 0},
                },
                .render_area = {.width = size.x, .height = size.y},
            });

            render_recorder.set_pipeline(*pipeline);

            render_recorder.set_index_buffer({
                .id = ti.get(indices).ids[0],
                .offset = 0,
                .index_type = daxa::IndexType::uint32,
            });

            render_recorder.push_constant(MyPushConstant{
                .my_vertex_ptr = ti.device.device_address(ti.get(vertices).ids[0]).value(),
                .ubo_ptr = ti.device.device_address(ti.get(uniform_buffer).ids[0]).value(),
            });

            render_recorder.draw_indexed({.index_count = 36});
            ti.recorder = std::move(render_recorder).end_renderpass();
        },
        .name = "draw mesh",
    });
}

void update_uniform_buffer(daxa::Device& device, daxa::BufferId uniform_buffer_id, float time, float aspect_ratio) {
    UniformBufferObject ubo{};
    ubo.model = glm::rotate(glm::mat4(1.0f), time, glm::vec3(0.0f, 1.0f, 0.0f));
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f),
                           glm::vec3(0.0f, 0.0f, 0.0f),
                           glm::vec3(0.0f, 1.0f, 0.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f), aspect_ratio, 0.1f, 10.0f);

    // Vulkan clip space Y is inverted, so invert proj matrix's Y here:
    ubo.proj[1][1] *= -1;

    auto* ptr = device.buffer_host_address_as<UniformBufferObject>(uniform_buffer_id).value();
    *ptr = ubo;
}

daxa::ImageId create_z_buffer(daxa::Device& device, daxa::Swapchain const& swapchain) {
    auto size = swapchain.get_surface_extent();
    return device.create_image({
        .format = daxa::Format::D32_SFLOAT,
        .size = {size.x, size.y, 1},
        .usage = daxa::ImageUsageFlagBits::DEPTH_STENCIL_ATTACHMENT,
        .name = "z-buffer",
    });
}

int main(int argc, char const* argv[]) {
    (void)argc;
    (void)argv;

    auto window = AppWindow("Hur Dur", 1600, 900);

    daxa::Instance instance = daxa::create_instance({
    });

    daxa::Device device = instance.create_device_2(instance.choose_device({}, {}));

    daxa::Swapchain swapchain = device.create_swapchain({
        .native_window = window.get_native_handle(),
        .native_window_platform = window.get_native_platform(),
        .present_mode = daxa::PresentMode::FIFO,
        .image_usage = daxa::ImageUsageFlagBits::TRANSFER_DST,
        .name = "my swapchain",
    });

    daxa::PipelineManager pipeline_manager = daxa::PipelineManager({
        .device = device,
        .root_paths = {
            DAXA_SHADER_INCLUDE_DIR,
            "C:/dev/Engine_project/shaders",
        },
        .default_language = std::optional{daxa::ShaderLanguage::GLSL},
        .default_enable_debug_info = std::optional{true},
        .name = "my pipeline manager",
    });

    std::shared_ptr<daxa::RasterPipeline> pipeline;
    {
        auto result = pipeline_manager.add_raster_pipeline2({
            .vertex_shader_info = daxa::ShaderCompileInfo2{.source = daxa::ShaderFile{"main.vert.glsl"}},
            .fragment_shader_info = daxa::ShaderCompileInfo2{.source = daxa::ShaderFile{"main.frag.glsl"}},
            .color_attachments = {{.format = swapchain.get_format()}},
            .depth_test = daxa::DepthTestInfo{
                .depth_attachment_format = daxa::Format::D32_SFLOAT,
                .enable_depth_write = true,                      // enable writing to depth buffer
                .depth_test_compare_op = daxa::CompareOp::LESS_OR_EQUAL,
                .min_depth_bounds = 0.0f,
                .max_depth_bounds = 1.0f,
            },
            .raster = daxa::RasterizerInfo{
                .face_culling = daxa::FaceCullFlagBits::BACK_BIT, // Optional but recommended
                .front_face_winding = daxa::FrontFaceWinding::COUNTER_CLOCKWISE, // Adjust to match your index winding
            },
            .push_constant_size = sizeof(MyPushConstant),
            .name = "my pipeline",
        });

        if (result.is_err()) {
            std::cerr << result.message() << std::endl;
            return -1;
        }
        pipeline = result.value();
    }

    auto uniform_buffer_id = device.create_buffer({
        .size = sizeof(UniformBufferObject),
        .allocate_info = daxa::MemoryFlagBits::HOST_ACCESS_RANDOM,
        .name = "Uniform buffer MVP",
    });

    auto vertex_buffer_id = device.create_buffer({
        .size = sizeof(MyVertex) * 8,
        .name = "my vertex data",
    });

    auto index_buffer_id = device.create_buffer({
        .size = sizeof(uint32_t) * 36,
        .name = "index buffer",
    });

    auto task_swapchain_image = daxa::TaskImage{ {.swapchain_image = true, .name = "swapchain image"} };

    // Create the z-buffer
    auto z_buffer_id = create_z_buffer(device, swapchain);

    auto task_z_buffer = daxa::TaskImage({
        .initial_images = {.images = std::span{&z_buffer_id, 1}},
        .name = "task depth image",
    });

    auto task_uniform_buffer = daxa::TaskBuffer({
        .initial_buffers = {.buffers = std::span{&uniform_buffer_id, 1}},
        .name = "task uniform buffer",
    });

    auto task_vertex_buffer = daxa::TaskBuffer({
        .initial_buffers = {.buffers = std::span{&vertex_buffer_id, 1}},
        .name = "task vertex buffer",
    });

    auto task_index_buffer = daxa::TaskBuffer({
        .initial_buffers = {.buffers = std::span{&index_buffer_id, 1}},
        .name = "task index buffer",
    });

    auto loop_task_graph = daxa::TaskGraph({
        .device = device,
        .swapchain = swapchain,
        .name = "loop",
        });

    loop_task_graph.use_persistent_buffer(task_vertex_buffer);
    loop_task_graph.use_persistent_buffer(task_index_buffer);
    loop_task_graph.use_persistent_buffer(task_uniform_buffer);
    loop_task_graph.use_persistent_image(task_z_buffer);
    loop_task_graph.use_persistent_image(task_swapchain_image);
    draw_mesh_task(loop_task_graph, pipeline, task_vertex_buffer, task_index_buffer, task_z_buffer, task_uniform_buffer, task_swapchain_image);

    loop_task_graph.submit({});
    // And tell the task graph to do the present step.
    loop_task_graph.present({});
    // Finally, we complete the task graph, which essentially compiles the
    // dependency graph between tasks, and inserts the most optimal synchronization!
    loop_task_graph.complete({});

    UniformBufferObject ubo{
    .model = glm::mat4(1.0f),
    .view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f),
                        glm::vec3(0.0f, 0.0f, 0.0f),
                        glm::vec3(0.0f, 0.0f, 1.0f)),
    .proj = glm::perspective(glm::radians(45.0f),
                             1600.0f / 900.0f,
                             0.1f, 10.0f),
    };
    ubo.proj[1][1] *= -1;  // Vulkan clip space correction

    {
        auto upload_task_graph = daxa::TaskGraph({
            .device = device,
            .name = "upload",
        });

        upload_task_graph.use_persistent_buffer(task_vertex_buffer);
        upload_task_graph.use_persistent_buffer(task_index_buffer);
        upload_task_graph.use_persistent_buffer(task_uniform_buffer);

        upload_mesh_data_task(upload_task_graph, task_vertex_buffer, task_index_buffer);
        upload_uniform_buffer_task(upload_task_graph, task_uniform_buffer, ubo);

        upload_task_graph.submit({});
        upload_task_graph.complete({});
        upload_task_graph.execute({});
    }

    // Main loop
    while (!window.should_close()) {
        window.update();

        if (window.swapchain_out_of_date) {
            swapchain.resize();
            window.swapchain_out_of_date = false;

            // Recreate our buffers
            device.destroy_image(z_buffer_id);
            z_buffer_id = create_z_buffer(device, swapchain);
            task_z_buffer.set_images({ .images = std::span{&z_buffer_id, 1} });
        }

        float time = static_cast<float>(glfwGetTime());
        float aspect_ratio = static_cast<float>(window.width) / window.height;
        update_uniform_buffer(device, uniform_buffer_id, time, aspect_ratio);

        auto swapchain_image = swapchain.acquire_next_image();
        if (swapchain_image.is_empty()) {   
            continue;
        }

        task_swapchain_image.set_images({ .images = std::span{&swapchain_image, 1} });

        loop_task_graph.execute({});
        device.collect_garbage();
    }

    device.destroy_image(z_buffer_id);
    device.destroy_buffer(uniform_buffer_id);
    device.destroy_buffer(vertex_buffer_id);
    device.destroy_buffer(index_buffer_id);


    device.wait_idle();
    device.collect_garbage();

    return 0;
}