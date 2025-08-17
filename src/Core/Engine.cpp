#include "Engine.h"

#include "window.h"
#include "shared.inl"

#include "Renderer/Meshes/DrawableMesh.h"
#include "Renderer/Meshes/DrawGroup.h"
#include "Renderer/Meshes/ManagedMesh.h"
#include "Renderer/Textures/TextureHandle.h"
#include "Renderer/Renderer.h"
#include "Renderer/MeshManager.h"

#include "Core/Camera.h"

#include "Engine/InputSystem/InputSystem.h"

#include "Tools/Model_loader.h"

#include "ECS/ECS.h"

#include "Transform/Transform_component.h"

#include <daxa/daxa.hpp>
#include <daxa/utils/pipeline_manager.hpp>
#include <daxa/utils/task_graph.hpp>

#include <iostream>

constexpr float MAX_DELTA_TIME = 0.1f;

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
                .face_culling = daxa::FaceCullFlagBits::BACK_BIT,
                .front_face_winding = daxa::FrontFaceWinding::COUNTER_CLOCKWISE,
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
    renderer.registerDrawGroup(std::move(drawGroup));

    BulkTextureUploadManager uploadManager;

    std::vector<std::unique_ptr<TextureHandle>> textures;
    std::vector<daxa::ImageViewId> views;

    std::vector<Entity> testEntities;

    ecs::entityManager.registerComponentManager<ManagedMesh>();    

    ecs::entityManager.registerComponentManager<TransformComponent>();
    ecs::entityManager.registerSystem<TransformSystem>(device, renderer, ecs::entityManager.getComponentManager<ManagedMesh>());

    {
        GLTF_Loader loader;
        loader.OpenFile("C:/dev/Vulkan_engine/assets/Sponza_glTF/Sponza.gltf");    //"C:/dev/Engine_project/assets/Sponza_glTF/Sponza.gltf"
        loader.LoadModel();

        for (int model_i = 0; model_i < loader.modelData.size(); ++model_i) {
            for (int prim_i = 0; prim_i < loader.modelData[model_i].primitives.size(); ++prim_i) {
                testEntities.push_back(EntityManager::createEntity());

                if (loader.modelData[model_i].primitives[prim_i].albedo.has_value()) {
                    textures.push_back(std::make_unique<TextureHandle>(device));
                    std::string debug_name = "Sponza_" + std::to_string(model_i) + "_" + std::to_string(prim_i);
                    textures.back()->stream_texture_from_data(loader.modelData[model_i].primitives[prim_i].albedo.value(), debug_name, uploadManager);
                }
            }
        }

        views = uploadManager.bulkUploadTextures(meshManager.upload_task_graph, "Sponza ");
        for (int model_i = 0; model_i < loader.modelData.size(); ++model_i) {
            for (int prim_i = 0; prim_i < loader.modelData[model_i].primitives.size(); ++prim_i) {
                ecs::getComponentManager<ManagedMesh>().addComponent(testEntities[model_i * loader.modelData.size() + prim_i], ManagedMesh(loader, model_i, prim_i, meshManager, renderer.drawGroups[0], renderer, {views[model_i * loader.modelData.size() + prim_i], sampler}));
                ecs::getComponentManager<TransformComponent>().addComponent(testEntities[model_i * loader.modelData.size() + prim_i], TransformComponent(ecs::entityManager, glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.01f)));
            }
        }
    }

    // int grid_size = 5;
	// float spacing = 0.08f;
    
    //{

    //    for (int x = 0; x < grid_size; ++x) {
    //        for (int y = 0; y < grid_size; ++y) {
    //            for (int z = 0; z < grid_size; ++z) {
    //                glm::vec3 position = glm::vec3(
    //                    (x - grid_size / 2) * spacing,
    //                    (y - grid_size / 2) * spacing,
    //                    (z - grid_size / 2) * spacing
    //                );
    //                int cubeIndex = x * grid_size * grid_size + y * grid_size + z; // Unique speed per instance
    //                
    //                if (cubeIndex != 0) {
    //                    testEntities.push_back(EntityManager::createEntity());
    //                    ecs::getComponent<ManagedMesh>(testEntities[0])->instantiate(testEntities[cubeIndex], meshComponenetManager, { view1, sampler });
    //                    ecs::getComponentManager<TransformComponent>().addComponent(testEntities[cubeIndex], TransformComponent(ecs::entityManager, position, glm::vec3(0.0f), glm::vec3(1.0f)));
    //                } else {
    //                    ecs::getComponent<TransformComponent>(testEntities[0])->setPosition(position);
    //                }
    //            }
    //        }
    //    }
    //}


    renderer.drawGroups[0].uploadBuffers(meshManager.upload_task_graph);

    meshManager.submit_upload_task_graph();
    renderer.submit_task_graph();

    Camera camera;
    camera.update_vectors();

    auto* glfw_window = window.get_glfw_window();
    window.camera_ptr = &camera;
    glfwSetCursorPosCallback(glfw_window, InputSystem::mouse_callback);

    window.set_mouse_capture(true);

    auto last_frame_time = static_cast<float>(glfwGetTime());
    ecs::updateSystems();

    // Main loop
    while (!window.should_close()) {
        auto current_time = static_cast<float>(glfwGetTime());

        float delta_time = current_time - last_frame_time;
        if (delta_time > MAX_DELTA_TIME) delta_time = MAX_DELTA_TIME;

        last_frame_time = current_time;

        window.update();

        InputSystem::process_input(window.get_glfw_window(), camera, delta_time);
        
        renderer.startFrame(camera);

        // ------------------------------------------------------ Goofy ahh test stuff ------------------------------------------------------
        //for (int x = 0; x < grid_size; ++x) {
        //    for (int y = 0; y < grid_size; ++y) {
        //        for (int z = 0; z < grid_size; ++z) {
        //            glm::vec3 position = glm::vec3(
        //                (x - grid_size / 2) * spacing,
        //                (y - grid_size / 2) * spacing,
        //                (z - grid_size / 2) * spacing
        //            );
        //            int cubeIndex = x * grid_size * grid_size + y * grid_size + z;
        //            float speed = 0.04f * static_cast<float>(cubeIndex); // Unique speed per instance
        //            float angle = current_time * speed;
        //            ecs::getComponent<TransformComponent>(testEntities[cubeIndex])->setRotation(glm::vec3(0.0f, angle, 0.0f));
        //        }
        //    }
        //}
        // ------------------------------------------------------- Goofy ahh test stuff ------------------------------------------------------
        ecs::updateSystems();

        renderer.endFrame();
    }

    for (auto& texture : textures) {
        texture->cleanup();
    }

    device.destroy_sampler(sampler);

    renderer.cleanup();


    device.wait_idle();
    device.collect_garbage();

    return 0;
}