#include "window.h"
#include "shared.inl"

#include "Renderer/Drawable.h"
#include "Renderer/DrawGroup.h"

#include "Core/Camera.h"
#include "Core/InputSystem.h"

#include <daxa/daxa.hpp>
#include <daxa/utils/pipeline_manager.hpp>
#include <daxa/utils/task_graph.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <iostream>

#define TEXTURE_PATH "C:/dev/Engine_project/assets/textures/"

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
    DrawGroup& drawGroup,
    daxa::TaskImageView z_buffer,
    daxa::TaskBufferView uniform_buffer,
    daxa::TaskImageView render_target,
    bool clear = false
) {
    std::vector<daxa::TaskAttachmentInfo> attachments;

    // Shared resources
    attachments.push_back(daxa::inl_attachment(daxa::TaskBufferAccess::VERTEX_SHADER_READ, uniform_buffer));
    attachments.push_back(daxa::inl_attachment(daxa::TaskImageAccess::COLOR_ATTACHMENT, daxa::ImageViewType::REGULAR_2D, render_target));
    attachments.push_back(daxa::inl_attachment(daxa::TaskImageAccess::DEPTH_ATTACHMENT, daxa::ImageViewType::REGULAR_2D, z_buffer));

    // Add each drawable's vertex/index/instance buffers
    for (auto& drawable : drawGroup.meshes) {
        attachments.push_back(daxa::inl_attachment(daxa::TaskBufferAccess::VERTEX_SHADER_READ, drawable.task_vertex_buffer));
        attachments.push_back(daxa::inl_attachment(daxa::TaskBufferAccess::VERTEX_SHADER_READ, drawable.task_instance_buffer));
        attachments.push_back(daxa::inl_attachment(daxa::TaskBufferAccess::INDEX_READ, drawable.task_index_buffer));
    }

    tg.add_task({
        .attachments = attachments,
        .task = [=](daxa::TaskInterface ti) {
            auto const size = ti.device.info(ti.get(render_target).ids[0]).value().size;

            daxa::RenderCommandRecorder render_recorder = std::move(ti.recorder).begin_renderpass({
                .color_attachments = std::array{
                    daxa::RenderAttachmentInfo{
                        .image_view = ti.get(render_target).view_ids[0],
                        .load_op = clear ? daxa::AttachmentLoadOp::CLEAR : daxa::AttachmentLoadOp::LOAD,
                        .clear_value = std::array<daxa::f32, 4>{0.1f, 0.0f, 0.5f, 1.0f},
                    },
                },
                .depth_attachment = daxa::RenderAttachmentInfo{
                    .image_view = ti.get(z_buffer).view_ids[0],
                    .load_op = clear ? daxa::AttachmentLoadOp::CLEAR : daxa::AttachmentLoadOp::LOAD,
                    .clear_value = daxa::DepthValue{1.0f, 0},
                },
                .render_area = {.width = size.x, .height = size.y},
            });

            render_recorder.set_pipeline(*drawGroup.pipeline);

            for (auto const& drawableMesh : drawGroup.meshes) {
                render_recorder.set_index_buffer({
                    .id = ti.get(drawableMesh.task_index_buffer).ids[0],
                    .offset = 0,
                    .index_type = daxa::IndexType::uint32,
                });

                render_recorder.push_constant(MyPushConstant{
                    .my_vertex_ptr = ti.device.device_address(ti.get(drawableMesh.task_vertex_buffer).ids[0]).value(),
                    .ubo_ptr = ti.device.device_address(ti.get(uniform_buffer).ids[0]).value(),
                    .instance_buffer_ptr = ti.device.device_address(ti.get(drawableMesh.task_instance_buffer).ids[0]).value(),
                });

                render_recorder.draw_indexed({
                    .index_count = drawableMesh.index_count,
                    .instance_count = static_cast<uint32_t>(drawableMesh.instance_data.size()),
                });
            }
            ti.recorder = std::move(render_recorder).end_renderpass();
        },
        .name = "draw mesh",
    });
}

void update_uniform_buffer(daxa::Device& device, daxa::BufferId uniform_buffer_id, Camera camera, float time, float aspect_ratio) {
    UniformBufferObject ubo{};
    ubo.view = camera.get_view_matrix();
    ubo.proj = camera.get_projection(aspect_ratio);

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

// --------------------------------------- TODO: abstractify these ---------------------------------------
struct TextureDataHandle {
    int width;
    int height;
    int channels;
    std::string name;

    daxa::ImageId image;
    daxa::TaskImage task_texture_image;
    daxa::BufferId texture_staging_buffer;
    daxa::TaskBuffer task_texture_staging;
};

TextureDataHandle stream_texture_from_memory(daxa::Device device, std::string fileName) {
    int width, height, channels;
    std::string path = "C:/dev/Engine_project/assets/textures/" + fileName;

    stbi_uc* pixels = stbi_load(path.c_str(), &width, &height, &channels, 4);
    if (!pixels) { std::cerr << "stb_image failed\n"; }

    uint32_t size_bytes = width * height * 4;

    daxa::ImageId image = device.create_image({
        .format = daxa::Format::R8G8B8A8_UNORM,
        .size = {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1},
        .usage = daxa::ImageUsageFlagBits::TRANSFER_DST | daxa::ImageUsageFlagBits::SHADER_SAMPLED,
        .name = fileName + " texture image"
    });

    daxa::TaskImage task_texture_image = daxa::TaskImage({
        .initial_images = {.images = std::span{&image, 1}},
        .name = fileName + " texutre task image"
    });

    daxa::BufferId texture_staging_buffer = device.create_buffer({
        .size = size_bytes,
        .allocate_info = daxa::MemoryFlagBits::HOST_ACCESS_SEQUENTIAL_WRITE,
        .name = fileName + " texture staging buffer"
    });

    daxa::TaskBuffer task_texture_staging = daxa::TaskBuffer({
        .initial_buffers = {.buffers = std::span{&texture_staging_buffer, 1} },
        .name = fileName + " staging task buffer"
    });

    {
        auto* ptr = device.buffer_host_address_as<std::byte>(task_texture_staging.get_state().buffers[0]).value();
        std::memcpy(ptr, pixels, size_bytes);
    }
    stbi_image_free(pixels);

    return {
		.width = width, .height = height, .channels = channels,
        .name = fileName,
        .image = image,
        .task_texture_image = task_texture_image,
        .texture_staging_buffer = texture_staging_buffer,
        .task_texture_staging = task_texture_staging
    };
}

daxa::ImageViewId load_texture(daxa::TaskGraph loop_task_graph, TextureDataHandle textureData) {
    loop_task_graph.use_persistent_buffer(textureData.task_texture_staging);
    loop_task_graph.use_persistent_image(textureData.task_texture_image);

    loop_task_graph.add_task({
        .attachments = {
            daxa::inl_attachment(daxa::TaskBufferAccess::TRANSFER_READ, textureData.task_texture_staging.view()),
            daxa::inl_attachment(daxa::TaskImageAccess::TRANSFER_WRITE, textureData.task_texture_image.view())
        },
        .task = [=](daxa::TaskInterface ti) {
            ti.recorder.pipeline_barrier_image_transition({
                .src_access = daxa::AccessConsts::NONE,
                .dst_access = daxa::AccessConsts::TRANSFER_WRITE,
                .src_layout = daxa::ImageLayout::UNDEFINED,
                .dst_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
                .image_slice = {
                    .base_mip_level = 0,
                    .level_count = 1,
                    .base_array_layer = 0,
                    .layer_count = 1
                },
                .image_id = ti.get(textureData.task_texture_image).ids[0],
            }),
            ti.recorder.copy_buffer_to_image({
                .buffer = ti.get(textureData.task_texture_staging).ids[0],
                .image = ti.get(textureData.task_texture_image).ids[0],
                .image_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
                .image_extent = {static_cast<uint32_t>(textureData.width), static_cast<uint32_t>(textureData.height), 1}
            }),
            ti.recorder.pipeline_barrier_image_transition({
                .src_access = daxa::AccessConsts::TRANSFER_WRITE,
                .dst_access = daxa::AccessConsts::FRAGMENT_SHADER_READ,
                .src_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
                .dst_layout = daxa::ImageLayout::READ_ONLY_OPTIMAL,
                .image_slice = {
                    .base_mip_level = 0,
                    .level_count = 1,
                    .base_array_layer = 0,
                    .layer_count = 1
                },
                .image_id = ti.get(textureData.task_texture_image).ids[0],
            });
        },
        .name = textureData.name + " upload"
        });

    return textureData.image.default_view();
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
            .vertex_shader_info = daxa::ShaderCompileInfo2{.source = daxa::ShaderFile{"main.vert.glsl"}, .defines = { {"DAXA_SHADER", "1"}, {"GLSL", "1"}} },
            .fragment_shader_info = daxa::ShaderCompileInfo2{.source = daxa::ShaderFile{"main.frag.glsl"}, .defines = { {"DAXA_SHADER", "1"}, {"GLSL", "1"}} },
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

    auto task_uniform_buffer = daxa::TaskBuffer({
        .initial_buffers = {.buffers = std::span{&uniform_buffer_id, 1}},
        .name = "task uniform buffer",
    });
    // Create the z-buffer
    auto z_buffer_id = create_z_buffer(device, swapchain);

    auto task_z_buffer = daxa::TaskImage({
        .initial_images = {.images = std::span{&z_buffer_id, 1}},
        .name = "task depth image",
    });

    auto task_swapchain_image = daxa::TaskImage{ {.swapchain_image = true, .name = "swapchain image"} };

    auto loop_task_graph = daxa::TaskGraph({
        .device = device,
        .swapchain = swapchain,
        .name = "loop",
    });

    // ------------------------------------------------------------------------

    TextureDataHandle texture1 = stream_texture_from_memory(device, "test_texture.jpg");

    daxa::ImageViewId view1 = load_texture(loop_task_graph, texture1);

    TextureDataHandle texture2 = stream_texture_from_memory(device, "test_texture2.jpg");

    daxa::ImageViewId view2 = load_texture(loop_task_graph, texture2);

    daxa::SamplerId sampler = device.create_sampler({
        .magnification_filter = daxa::Filter::LINEAR,
		.minification_filter = daxa::Filter::LINEAR,
        .mipmap_filter = daxa::Filter::LINEAR,
        .address_mode_u = daxa::SamplerAddressMode::REPEAT,
        .address_mode_v = daxa::SamplerAddressMode::REPEAT,
    });

   // ------------------------------------------------------------------------

    UniformBufferObject ubo{
        .model = glm::mat4(1.0f),
        .view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f),
                            glm::vec3(0.0f, 0.0f, 0.0f),
                            glm::vec3(0.0f, 0.0f, 1.0f)),
        .proj = glm::perspective(glm::radians(45.0f),
                                 1600.0f / 900.0f,
                                 0.1f, 10.0f),
    };

    DrawGroup drawGroup2(device, pipeline, "My Other DrawGroup");
    drawGroup2.add_mesh<8, 36>(
        std::array<MyVertex, 8>{
            MyVertex{ .position = {-0.5f, -0.5f, -0.5f}, .uv = {0, 0} },
            MyVertex{ .position = {+0.5f, -0.5f, -0.5f}, .uv = {1, 0} },
            MyVertex{ .position = {+0.5f, +0.5f, -0.5f}, .uv = {1, 1} },
            MyVertex{ .position = {-0.5f, +0.5f, -0.5f}, .uv = {0, 1} },
            MyVertex{ .position = {-0.5f, -0.5f, +0.5f}, .uv = {0, 0} },
            MyVertex{ .position = {+0.5f, -0.5f, +0.5f}, .uv = {1, 0} },
            MyVertex{ .position = {+0.5f, +0.5f, +0.5f}, .uv = {1, 1} },
            MyVertex{ .position = {-0.5f, +0.5f, +0.5f}, .uv = {0, 1} },
    },
        std::array<uint32_t, 36>{
            // front face
            0, 2, 1, 2, 0, 3,
            // right face
            1, 6, 5, 6, 1, 2,
            // back face
            5, 7, 4, 7, 5, 6,
            // left face
            4, 3, 0, 3, 4, 7,
            // top face
            3, 6, 2, 6, 3, 7,
            // bottom face
            4, 1, 5, 1, 4, 0,
    },
        "Cube"
    );
    drawGroup2.use_in_loop_task_graph(0, loop_task_graph);

    DrawGroup drawGroup(device, pipeline, "My DrawGroup");
    drawGroup.add_mesh<8, 36>(
        std::array<MyVertex, 8>{
            MyVertex{ .position = {-0.5f, -0.5f, -0.5f}, .uv = {0, 0} },
            MyVertex{ .position = {+0.5f, -0.5f, -0.5f}, .uv = {1, 0} },
            MyVertex{ .position = {+0.5f, +0.5f, -0.5f}, .uv = {1, 1} },
            MyVertex{ .position = {-0.5f, +0.5f, -0.5f}, .uv = {0, 1} },
            MyVertex{ .position = {-0.5f, -0.5f, +0.5f}, .uv = {0, 0} },
            MyVertex{ .position = {+0.5f, -0.5f, +0.5f}, .uv = {1, 0} },
            MyVertex{ .position = {+0.5f, +0.5f, +0.5f}, .uv = {1, 1} },
            MyVertex{ .position = {-0.5f, +0.5f, +0.5f}, .uv = {0, 1} },
    },
        std::array<uint32_t, 36>{
            // front face
            0, 2, 1, 2, 0, 3,
            // right face
            1, 6, 5, 6, 1, 2,
            // back face
            5, 7, 4, 7, 5, 6,
            // left face
            4, 3, 0, 3, 4, 7,
            // top face
            3, 6, 2, 6, 3, 7,
            // bottom face
            4, 1, 5, 1, 4, 0,
        },
        "Cube"
    );
    drawGroup.use_in_loop_task_graph(0, loop_task_graph);

    drawGroup.add_mesh<24, 36>(
        std::array<MyVertex, 24>{
            // Front face (Z+)
            MyVertex{ .position = {-0.5f, -0.5f, +0.5f}, .uv = {0.0f, 0.0f} },   // 0
            MyVertex{ .position = {+0.5f, -0.5f, +0.5f}, .uv = {1.0f, 0.0f} },   // 1
            MyVertex{ .position = {+0.5f, +0.5f, +0.5f}, .uv = {1.0f, 1.0f} },   // 2
            MyVertex{ .position = {-0.5f, +0.5f, +0.5f}, .uv = {0.0f, 1.0f} },   // 3

            // Back face (Z-)
            MyVertex{ .position = {+0.5f, -0.5f, -0.5f}, .uv = {0.0f, 0.0f} },   // 4
            MyVertex{ .position = {-0.5f, -0.5f, -0.5f}, .uv = {1.0f, 0.0f} },   // 5
            MyVertex{ .position = {-0.5f, +0.5f, -0.5f}, .uv = {1.0f, 1.0f} },   // 6
            MyVertex{ .position = {+0.5f, +0.5f, -0.5f}, .uv = {0.0f, 1.0f} },   // 7

            // Left face (X-)
            MyVertex{ .position = {-0.5f, -0.5f, -0.5f}, .uv = {0.0f, 0.0f} },   // 8
            MyVertex{ .position = {-0.5f, -0.5f, +0.5f}, .uv = {1.0f, 0.0f} },   // 9
            MyVertex{ .position = {-0.5f, +0.5f, +0.5f}, .uv = {1.0f, 1.0f} },   // 10
            MyVertex{ .position = {-0.5f, +0.5f, -0.5f}, .uv = {0.0f, 1.0f} },   // 11

            // Right face (X+)
            MyVertex{ .position = {+0.5f, -0.5f, +0.5f}, .uv = {0.0f, 0.0f} },   // 12
            MyVertex{ .position = {+0.5f, -0.5f, -0.5f}, .uv = {1.0f, 0.0f} },   // 13
            MyVertex{ .position = {+0.5f, +0.5f, -0.5f}, .uv = {1.0f, 1.0f} },   // 14
            MyVertex{ .position = {+0.5f, +0.5f, +0.5f}, .uv = {0.0f, 1.0f} },   // 15

            // Top face (Y+)
            MyVertex{ .position = {-0.5f, +0.5f, +0.5f}, .uv = {0.0f, 0.0f} },   // 16rz
            MyVertex{ .position = {+0.5f, +0.5f, +0.5f}, .uv = {1.0f, 0.0f} },   // 17
            MyVertex{ .position = {+0.5f, +0.5f, -0.5f}, .uv = {1.0f, 1.0f} },   // 18
            MyVertex{ .position = {-0.5f, +0.5f, -0.5f}, .uv = {0.0f, 1.0f} },   // 19

            // Bottom face (Y-)
            MyVertex{ .position = {-0.5f, -0.5f, -0.5f}, .uv = {0.0f, 0.0f} },   // 20
            MyVertex{ .position = {+0.5f, -0.5f, -0.5f}, .uv = {1.0f, 0.0f} },   // 21
            MyVertex{ .position = {+0.5f, -0.5f, +0.5f}, .uv = {1.0f, 1.0f} },   // 22
            MyVertex{ .position = {-0.5f, -0.5f, +0.5f}, .uv = {0.0f, 1.0f} },   // 23
    },
        std::array<uint32_t, 36>{
            // Front face
            0, 1, 2, 0, 2, 3,
            // Back face
            4, 5, 6, 4, 6, 7,
            // Left face
            8, 9, 10, 8, 10, 11,
            // Right face
            12, 13, 14, 12, 14, 15,
            // Top face
            16, 17, 18, 16, 18, 19,
            // Bottom face
            20, 21, 22, 20, 22, 23,
    },
        "Other cube"
    );
    drawGroup.use_in_loop_task_graph(1, loop_task_graph);

    int grid_size = 5;        // 5x5x5 grid of cubes
    float spacing = 2.0f;     // distance between cubes

    {

        for (int x = 0; x < grid_size; ++x) {
            for (int y = 0; y < grid_size; ++y) {
                for (int z = 0; z < grid_size; ++z) {
                    glm::vec3 position = glm::vec3(
                        (x - grid_size / 2) * spacing,
                        (y - grid_size / 2) * spacing,
                        (z - grid_size / 2) * spacing
                    );

                    PerInstanceData data;
                    data.model_matrix = glm::translate(glm::mat4(1.0f), position);
                    data.texture = view1;
                    data.tex_sampler = sampler;
                    drawGroup.meshes[0].instance_data.push_back(data);
                }
            }
        }

        auto* ptr = device.buffer_host_address_as<PerInstanceData>(drawGroup.meshes[0].instance_buffer_id).value();
        memcpy(ptr, drawGroup.meshes[0].instance_data.data(), drawGroup.meshes[0].instance_data.size() * sizeof(PerInstanceData));

    }

    {

        for (int x = 0; x < grid_size; ++x) {
            for (int y = 0; y < grid_size; ++y) {
                for (int z = 0; z < grid_size; ++z) {
                    glm::vec3 position = glm::vec3(
                        (x - grid_size / 2) * spacing - 15.0f,
                        (y - grid_size / 2) * spacing,
                        (z - grid_size / 2) * spacing
                    );

                    PerInstanceData data;
                    data.model_matrix = glm::translate(glm::mat4(1.0f), position);
                    //data.texture = view2;
					data.tex_sampler = sampler;
                    drawGroup.meshes[1].instance_data.push_back(data);
                }
            }
        }

        auto* ptr = device.buffer_host_address_as<PerInstanceData>(drawGroup.meshes[1].instance_buffer_id).value();
        memcpy(ptr, drawGroup.meshes[1].instance_data.data(), drawGroup.meshes[1].instance_data.size() * sizeof(PerInstanceData));

    }

    {

        for (int x = 0; x < grid_size; ++x) {
            for (int y = 0; y < grid_size; ++y) {
                for (int z = 0; z < grid_size; ++z) {
                    glm::vec3 position = glm::vec3(
                        (x - grid_size / 2) * spacing,
                        (y - grid_size / 2) * spacing,
                        (z - grid_size / 2) * spacing - 15.0f
                    );

                    PerInstanceData data;
                    data.model_matrix = glm::translate(glm::mat4(1.0f), position);
                    data.texture = view1;
                    data.tex_sampler = sampler;
                    drawGroup2.meshes[0].instance_data.push_back(data);
                }
            }
        }

        auto* ptr = device.buffer_host_address_as<PerInstanceData>(drawGroup2.meshes[0].instance_buffer_id).value();
        memcpy(ptr, drawGroup2.meshes[0].instance_data.data(), drawGroup2.meshes[0].instance_data.size() * sizeof(PerInstanceData));

    }

    //otherCube.use_in_loop_task_graph(loop_task_graph);

    //drawGroup.push_back(otherCube);

    loop_task_graph.use_persistent_buffer(task_uniform_buffer);
    loop_task_graph.use_persistent_image(task_z_buffer);
    loop_task_graph.use_persistent_image(task_swapchain_image);
    draw_mesh_task(loop_task_graph, drawGroup2, task_z_buffer, task_uniform_buffer, task_swapchain_image, true);
    draw_mesh_task(loop_task_graph, drawGroup, task_z_buffer, task_uniform_buffer, task_swapchain_image, false);

    loop_task_graph.submit({});
    // And tell the task graph to do the present step.
    loop_task_graph.present({});
    // Finally, we complete the task graph, which essentially compiles the
    // dependency graph between tasks, and inserts the most optimal synchronization!
    loop_task_graph.complete({});

    Camera camera;
    camera.update_vectors();

    auto* glfw_window = window.get_glfw_window();
    //glfwSetWindowUserPointer(glfw_window, &camera);
    window.camera_ptr = &camera;
    glfwSetCursorPosCallback(glfw_window, InputSystem::mouse_callback);

    // Capture the mouse cursor (optional)
    window.set_mouse_capture(true);

    float last_frame_time = static_cast<float>(glfwGetTime());

    // Main loop
    while (!window.should_close()) {
        float current_time = static_cast<float>(glfwGetTime());
        float delta_time = current_time - last_frame_time;
        last_frame_time = current_time;

        window.update();

        InputSystem::process_input(window.get_glfw_window(), camera, delta_time);

        if (window.swapchain_out_of_date) {
            swapchain.resize();
            window.swapchain_out_of_date = false;

            // Recreate our buffers
            device.destroy_image(z_buffer_id);
            z_buffer_id = create_z_buffer(device, swapchain);
            task_z_buffer.set_images({ .images = std::span{&z_buffer_id, 1} });
        }

        float aspect_ratio = static_cast<float>(window.width) / window.height;
        update_uniform_buffer(device, uniform_buffer_id, camera, current_time, aspect_ratio);


        for (int x = 0; x < grid_size; ++x) {
            for (int y = 0; y < grid_size; ++y) {
                for (int z = 0; z < grid_size; ++z) {
                    glm::vec3 position = glm::vec3(
                        (x - grid_size / 2) * spacing,
                        (y - grid_size / 2) * spacing,
                        (z - grid_size / 2) * spacing
                    );
                    int cubeIndex = x * grid_size * grid_size + y * grid_size + z;
                    float speed = 0.2f + 0.1f * static_cast<float>(cubeIndex); // Unique speed per instance
                    float angle = current_time * speed;
                    drawGroup.meshes[0].instance_data[cubeIndex].model_matrix = glm::translate(glm::mat4(1.0f), position) * glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 1, 0));
                }
            }
        }
        auto* ptr = device.buffer_host_address_as<PerInstanceData>(drawGroup.meshes[0].instance_buffer_id).value();
        std::memcpy(ptr, drawGroup.meshes[0].instance_data.data(), drawGroup.meshes[0].instance_data.size() * sizeof(PerInstanceData));

        for (int x = 0; x < grid_size; ++x) {
            for (int y = 0; y < grid_size; ++y) {
                for (int z = 0; z < grid_size; ++z) {
                    glm::vec3 position = glm::vec3(
                        (x - grid_size / 2) * spacing - 15.0f,
                        (y - grid_size / 2) * spacing,
                        (z - grid_size / 2) * spacing
                    );
                    int cubeIndex = x * grid_size * grid_size + y * grid_size + z;
                    float speed = 0.2f + 0.1f * static_cast<float>(cubeIndex);
                    float angle = current_time * speed;
                    drawGroup.meshes[1].instance_data[cubeIndex].model_matrix = glm::translate(glm::mat4(1.0f), position) * glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 1, 0));
                }
            }
        }
        auto* other_ptr = device.buffer_host_address_as<PerInstanceData>(drawGroup.meshes[1].instance_buffer_id).value();
        std::memcpy(other_ptr, drawGroup.meshes[1].instance_data.data(), drawGroup.meshes[1].instance_data.size() * sizeof(PerInstanceData));

        auto swapchain_image = swapchain.acquire_next_image();

        task_swapchain_image.set_images({ .images = std::span{&swapchain_image, 1} });

        loop_task_graph.execute({});
        device.collect_garbage();
    }

    device.destroy_image(texture1.image);
    device.destroy_buffer(texture1.texture_staging_buffer);
    device.destroy_image(texture2.image);
    device.destroy_buffer(texture2.texture_staging_buffer);

    device.destroy_sampler(sampler);

    drawGroup.cleanup();
    drawGroup2.cleanup();

    device.destroy_image(z_buffer_id);
    device.destroy_buffer(uniform_buffer_id);


    device.wait_idle();
    device.collect_garbage();

    return 0;
}