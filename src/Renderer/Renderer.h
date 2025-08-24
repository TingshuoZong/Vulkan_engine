#pragma once

#include "window.h"
#include "mesh_rendering_shared.inl"
#include "skybox_rendering_shared.inl"

#include "Renderer/Skybox/Skybox.h"

#undef Drawable
#include "Renderer/Meshes/DrawGroup.h"

#include "Core/Camera.h"

#include <daxa/daxa.hpp>
#include <daxa/utils/pipeline_manager.hpp>
#include <daxa/utils/task_graph.hpp>

#include <Daxa/utils/imgui.hpp>
#include <imgui_impl_glfw.h>

constexpr const char* GLOBAL_SHADER_PATH = "C:/dev/Engine_project/shaders";
constexpr float V_FOV = 60.0f;

constexpr bool DEBUG_WINDOW = true;

struct Renderer {
    // TODO: implement a name to prevent daxa name conflicts
    GLFW_Window::AppWindow& window;

    daxa::Device& device;
    daxa::Instance& instance;

    daxa::Swapchain swapchain;
    daxa::PipelineManager pipeline_manager;

    Skybox skybox;
    std::vector<DrawGroup> drawGroups;

    daxa::BufferId mesh_uniform_buffer_id;
    daxa::BufferId skybox_uniform_buffer_id;

    daxa::TaskBuffer task_mesh_uniform_buffer;
    daxa::TaskBuffer task_skybox_uniform_buffer;
    
    daxa::ImageId z_buffer_id;
    daxa::TaskImage task_z_buffer;
    daxa::TaskImage task_swapchain_image;
    daxa::TaskGraph loop_task_graph;

    daxa::ImGuiRenderer imguiRenderer;

    Renderer(GLFW_Window::AppWindow& window, daxa::Device& device, daxa::Instance& instance);

    static void upload_uniform_buffer_task(daxa::TaskGraph& tg, daxa::TaskBufferView uniform_buffer, const meshRenderer::UniformBufferObject &ubo);

    void draw_mesh_task();
    void draw_skybox_task();

    void registerDrawGroup(DrawGroup&& drawGroup);

    static void update_mesh_uniform_buffer(const daxa::Device& device, daxa::BufferId uniform_buffer_id, Camera camera, float aspect_ratio);
    static void update_skybox_uniform_buffer(const daxa::Device& device, daxa::BufferId uniform_buffer_id, Camera camera, float aspect_ratio);

    void init();
    void submit_task_graph();
    void cleanup();

    void startFrame(const Camera& camera);
    void endFrame();
};
