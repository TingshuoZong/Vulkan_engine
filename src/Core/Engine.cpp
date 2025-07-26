#include "Engine.h"

#include "window.h"
#include "shared.inl"

#include "Renderer/DrawableMesh.h"
#include "Renderer/DrawGroup.h"
#include "Renderer/TextureHandle.h"
#include "Renderer/Renderer.h"
#include "Renderer/MeshManager.h"

#include "Core/Camera.h"
#include "Core/InputSystem.h"

#include "Tools/Model_loader.h"

#include <daxa/daxa.hpp>
#include <daxa/utils/pipeline_manager.hpp>
#include <daxa/utils/task_graph.hpp>

#include <iostream>

int init() {
    auto window = GLFW_Window::AppWindow("Hur Dur", 1600, 900);

    daxa::Instance instance = daxa::create_instance({});

    daxa::Device device = instance.create_device_2(instance.choose_device({}, {}));

    Renderer renderer = Renderer(window, device, instance);
    renderer.loop_task_graph = daxa::TaskGraph({
        .device = device,
        .swapchain = renderer.swapchain,
        .name = "loop",
    });
    renderer.init();

    std::shared_ptr<daxa::RasterPipeline> pipeline;
    {
        auto result = renderer.pipeline_manager.add_raster_pipeline2({
            .vertex_shader_info = daxa::ShaderCompileInfo2{.source = daxa::ShaderFile{"main.vert.glsl"}, .defines = { {"DAXA_SHADER", "1"}, {"GLSL", "1"}} },
            .fragment_shader_info = daxa::ShaderCompileInfo2{.source = daxa::ShaderFile{"main.frag.glsl"}, .defines = { {"DAXA_SHADER", "1"}, {"GLSL", "1"}} },
            .color_attachments = {{.format = renderer.swapchain.get_format()}},
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
            .push_constant_size = sizeof(PushConstant),
            .name = "my pipeline",
            });

        if (result.is_err()) {
            std::cerr << result.message() << std::endl;
            return -1;
        }
        pipeline = result.value();
    }
    TextureHandle texture1 = TextureHandle(device, "test_texture.jpg");
    texture1.stream_texture_from_memory();

    daxa::ImageViewId view1 = texture1.load_texture(renderer.loop_task_graph);

    TextureHandle texture2 = TextureHandle(device, "test_texture2.jpg");
    texture2.stream_texture_from_memory();

    daxa::ImageViewId view2 = texture2.load_texture(renderer.loop_task_graph);

    daxa::SamplerId sampler = device.create_sampler({
        .magnification_filter = daxa::Filter::LINEAR,
        .minification_filter = daxa::Filter::LINEAR,
        .mipmap_filter = daxa::Filter::LINEAR,
        .address_mode_u = daxa::SamplerAddressMode::REPEAT,
        .address_mode_v = daxa::SamplerAddressMode::REPEAT,
        });
// -----------------------------------------------------------------------------------------------------------------
    MeshManager meshManager(device);
    DrawGroup drawGroup(device, pipeline, "My DrawGroup");

    // {
    //     GLTF_Loader loader;
    //     loader.OpenFile("C:/dev/Engine_project/assets/Avocado_glTF/Avocado.gltf");
    //     loader.LoadModel();
    //     meshManager.add_mesh<loader.getModelData(0).vertexCount, loader.getModelData(0).indexCount>("Avocado");
    //     meshManager.upload_mesh_data_task(0, meshManager.upload_task_graph, loader.getModelData(0).vertices, loader.getModelData(0).indices);
    // }

    DrawGroup drawGroup2(device, pipeline, "My Other DrawGroup");

     meshManager.add_mesh<8, 36>("Cube");
     meshManager.upload_mesh_data_task(0, meshManager.upload_task_graph,
         std::array<Vertex, 8>{
             Vertex{ .position = {-0.5f, -0.5f, -0.5f}, .uv = {0, 0} },
             Vertex{ .position = {+0.5f, -0.5f, -0.5f}, .uv = {1, 0} },
             Vertex{ .position = {+0.5f, +0.5f, -0.5f}, .uv = {1, 1} },
             Vertex{ .position = {-0.5f, +0.5f, -0.5f}, .uv = {0, 1} },
             Vertex{ .position = {-0.5f, -0.5f, +0.5f}, .uv = {0, 0} },
             Vertex{ .position = {+0.5f, -0.5f, +0.5f}, .uv = {1, 0} },
             Vertex{ .position = {+0.5f, +0.5f, +0.5f}, .uv = {1, 1} },
             Vertex{ .position = {-0.5f, +0.5f, +0.5f}, .uv = {0, 1} },
         }, std::array<uint32_t, 36>{
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
         });

     drawGroup2.register_mesh(meshManager.get_mesh_ptr(0), renderer.loop_task_graph);

     meshManager.add_mesh<24, 366>("Cube 2");
     meshManager.upload_mesh_data_task(1, meshManager.upload_task_graph,
         std::array<Vertex, 24>{
             // Front face (Z+)
             Vertex{ .position = {-0.5f, -0.5f, +0.5f}, .uv = {0.0f, 0.0f} },   // 0
             Vertex{ .position = {+0.5f, -0.5f, +0.5f}, .uv = {1.0f, 0.0f} },   // 1
             Vertex{ .position = {+0.5f, +0.5f, +0.5f}, .uv = {1.0f, 1.0f} },   // 2
             Vertex{ .position = {-0.5f, +0.5f, +0.5f}, .uv = {0.0f, 1.0f} },   // 3

             // Back face (Z-)
             Vertex{ .position = {+0.5f, -0.5f, -0.5f}, .uv = {0.0f, 0.0f} },   // 4
             Vertex{ .position = {-0.5f, -0.5f, -0.5f}, .uv = {1.0f, 0.0f} },   // 5
             Vertex{ .position = {-0.5f, +0.5f, -0.5f}, .uv = {1.0f, 1.0f} },   // 6
             Vertex{ .position = {+0.5f, +0.5f, -0.5f}, .uv = {0.0f, 1.0f} },   // 7

             // Left face (X-)
             Vertex{ .position = {-0.5f, -0.5f, -0.5f}, .uv = {0.0f, 0.0f} },   // 8
             Vertex{ .position = {-0.5f, -0.5f, +0.5f}, .uv = {1.0f, 0.0f} },   // 9
             Vertex{ .position = {-0.5f, +0.5f, +0.5f}, .uv = {1.0f, 1.0f} },   // 10
             Vertex{ .position = {-0.5f, +0.5f, -0.5f}, .uv = {0.0f, 1.0f} },   // 11

             // Right face (X+)
             Vertex{ .position = {+0.5f, -0.5f, +0.5f}, .uv = {0.0f, 0.0f} },   // 12
             Vertex{ .position = {+0.5f, -0.5f, -0.5f}, .uv = {1.0f, 0.0f} },   // 13
             Vertex{ .position = {+0.5f, +0.5f, -0.5f}, .uv = {1.0f, 1.0f} },   // 14
             Vertex{ .position = {+0.5f, +0.5f, +0.5f}, .uv = {0.0f, 1.0f} },   // 15

             // Top face (Y+)
             Vertex{ .position = {-0.5f, +0.5f, +0.5f}, .uv = {0.0f, 0.0f} },   // 16
             Vertex{ .position = {+0.5f, +0.5f, +0.5f}, .uv = {1.0f, 0.0f} },   // 17
             Vertex{ .position = {+0.5f, +0.5f, -0.5f}, .uv = {1.0f, 1.0f} },   // 18
             Vertex{ .position = {-0.5f, +0.5f, -0.5f}, .uv = {0.0f, 1.0f} },   // 19

             // Bottom face (Y-)
             Vertex{ .position = {-0.5f, -0.5f, -0.5f}, .uv = {0.0f, 0.0f} },   // 20
             Vertex{ .position = {+0.5f, -0.5f, -0.5f}, .uv = {1.0f, 0.0f} },   // 21
             Vertex{ .position = {+0.5f, -0.5f, +0.5f}, .uv = {1.0f, 1.0f} },   // 22
             Vertex{ .position = {-0.5f, -0.5f, +0.5f}, .uv = {0.0f, 1.0f} },   // 23
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
         }
     );

     drawGroup2.register_mesh(meshManager.get_mesh_ptr(1), renderer.loop_task_graph);

     meshManager.add_mesh<8, 36>("Cube3");

     meshManager.upload_mesh_data_task(2, meshManager.upload_task_graph,
         std::array<Vertex, 8>{
             Vertex{ .position = {-0.5f, -0.5f, -0.5f}, .uv = {0, 0} },
             Vertex{ .position = {+0.5f, -0.5f, -0.5f}, .uv = {1, 0} },
             Vertex{ .position = {+0.5f, +0.5f, -0.5f}, .uv = {1, 1} },
             Vertex{ .position = {-0.5f, +0.5f, -0.5f}, .uv = {0, 1} },
             Vertex{ .position = {-0.5f, -0.5f, +0.5f}, .uv = {0, 0} },
             Vertex{ .position = {+0.5f, -0.5f, +0.5f}, .uv = {1, 0} },
             Vertex{ .position = {+0.5f, +0.5f, +0.5f}, .uv = {1, 1} },
             Vertex{ .position = {-0.5f, +0.5f, +0.5f}, .uv = {0, 1} },
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
         }
     );

     drawGroup.register_mesh(meshManager.get_mesh_ptr(2), renderer.loop_task_graph);

     meshManager.submit_upload_task_graph();

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

                     PerInstanceData data{};
                     data.model_matrix = glm::translate(glm::mat4(1.0f), position);
                     data.texture = view1;
                     data.tex_sampler = sampler;
                     drawGroup.meshes[0].lock()->instance_data.push_back(data);
                 }
             }
         }

         auto* ptr = device.buffer_host_address_as<PerInstanceData>(drawGroup.meshes[0].lock()->instance_buffer_id).value();
         memcpy(ptr, drawGroup.meshes[0].lock()->instance_data.data(), drawGroup.meshes[0].lock()->instance_data.size() * sizeof(PerInstanceData));

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

                     PerInstanceData data{};
                     data.model_matrix = glm::translate(glm::mat4(1.0f), position);
                     data.texture = view2;
                     data.tex_sampler = sampler;
                     drawGroup2.meshes[1].lock()->instance_data.push_back(data);
                 }
             }
         }

         auto* ptr = device.buffer_host_address_as<PerInstanceData>(drawGroup2.meshes[1].lock()->instance_buffer_id).value();
         memcpy(ptr, drawGroup2.meshes[1].lock()->instance_data.data(), drawGroup2.meshes[1].lock()->instance_data.size() * sizeof(PerInstanceData));

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

                     PerInstanceData data{};
                     data.model_matrix = glm::translate(glm::mat4(1.0f), position);
                     data.texture = view1;
                     data.tex_sampler = sampler;
                     drawGroup2.meshes[0].lock()->instance_data.push_back(data);
                 }
             }
         }

         auto* ptr = device.buffer_host_address_as<PerInstanceData>(drawGroup2.meshes[0].lock()->instance_buffer_id).value();
         memcpy(ptr, drawGroup2.meshes[0].lock()->instance_data.data(), drawGroup2.meshes[0].lock()->instance_data.size() * sizeof(PerInstanceData));
     }

	 // ----------------------------------------------------------------- //

    std::vector<DrawGroup> drawGroups;
    drawGroups.push_back(drawGroup);
    drawGroups.push_back(drawGroup2);
    renderer.drawGroups.push_back(drawGroup);
    renderer.drawGroups.push_back(drawGroup2);

    renderer.submit_task_graph();

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
        
        renderer.startFrame(camera);

        // ------------------------------------------------------ Goofy ahh test stuff ------------------------------------------------------
        // for (int x = 0; x < grid_size; ++x) {
        //     for (int y = 0; y < grid_size; ++y) {
        //         for (int z = 0; z < grid_size; ++z) {
        //             glm::vec3 position = glm::vec3(
        //                 (x - grid_size / 2) * spacing,
        //                 (y - grid_size / 2) * spacing,
        //                 (z - grid_size / 2) * spacing
        //             );
        //             int cubeIndex = x * grid_size * grid_size + y * grid_size + z;
        //             float speed = 0.2f + 0.1f * static_cast<float>(cubeIndex); // Unique speed per instance
        //             float angle = current_time * speed;
        //             drawGroup.meshes[0].lock()->instance_data[cubeIndex].model_matrix = glm::translate(glm::mat4(1.0f), position) * glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 1, 0));
        //         }
        //     }
        // }
        // auto* ptr = device.buffer_host_address_as<PerInstanceData>(drawGroup.meshes[0].lock()->instance_buffer_id).value();
        // std::memcpy(ptr, drawGroup.meshes[0].lock()->instance_data.data(), drawGroup.meshes[0].lock()->instance_data.size() * sizeof(PerInstanceData));
        //
        // for (int x = 0; x < grid_size; ++x) {
        //     for (int y = 0; y < grid_size; ++y) {
        //         for (int z = 0; z < grid_size; ++z) {
        //             glm::vec3 position = glm::vec3(
        //                 (x - grid_size / 2) * spacing - 15.0f,
        //                 (y - grid_size / 2) * spacing,
        //                 (z - grid_size / 2) * spacing
        //             );
        //             int cubeIndex = x * grid_size * grid_size + y * grid_size + z;
        //             float speed = 0.2f + 0.1f * static_cast<float>(cubeIndex);
        //             float angle = current_time * speed;
        //             drawGroup2.meshes[1].lock()->instance_data[cubeIndex].model_matrix = glm::translate(glm::mat4(1.0f), position) * glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 1, 0));
        //         }
        //     }
        // }
        // auto* other_ptr = device.buffer_host_address_as<PerInstanceData>(drawGroup2.meshes[1].lock()->instance_buffer_id).value();
        // std::memcpy(other_ptr, drawGroup2.meshes[1].lock()->instance_data.data(), drawGroup2.meshes[1].lock()->instance_data.size() * sizeof(PerInstanceData));
        // ------------------------------------------------------- Goofy ahh test stuff ------------------------------------------------------

        renderer.endFrame();
    }

    texture1.cleanup();
    texture2.cleanup();

    device.destroy_sampler(sampler);

    renderer.cleanup();


    device.wait_idle();
    device.collect_garbage();

    return 0;
}