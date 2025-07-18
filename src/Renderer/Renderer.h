#pragma once

#include "window.h"
#include "shared.inl"

#include "Renderer/Drawable.h"
#include "Renderer/DrawGroup.h"
#include "Renderer/TextureHandle.h"

#include "Core/Camera.h"
#include "Core/InputSystem.h"

#include <daxa/daxa.hpp>
#include <daxa/utils/pipeline_manager.hpp>
#include <daxa/utils/task_graph.hpp>

#include <iostream>

constexpr const char* GLOBAL_SHADER_PATH = "C:/dev/Engine_project/shaders";
constexpr float V_FOV = 60.0f;

struct Renderer {
    // TODO: implement a name to prevent daxa name conflicts
    AppWindow& window;

    daxa::Device& device;
    daxa::Instance& instance;

    daxa::Swapchain swapchain;
    daxa::PipelineManager pipeline_manager;

    std::vector<DrawGroup> drawGroups;

    daxa::BufferId uniform_buffer_id;
    daxa::TaskBuffer task_uniform_buffer;
    daxa::ImageId z_buffer_id;
    daxa::TaskImage task_z_buffer;
    daxa::TaskImage task_swapchain_image;
    daxa::TaskGraph loop_task_graph;

    Renderer(AppWindow& window, daxa::Device& device, daxa::Instance& instance);

    void upload_uniform_buffer_task(daxa::TaskGraph& tg, daxa::TaskBufferView uniform_buffer, UniformBufferObject ubo);

    void draw_mesh_task(DrawGroup& drawGroup, bool clear = false);

    void update_uniform_buffer(daxa::Device& device, daxa::BufferId uniform_buffer_id, Camera camera, float aspect_ratio);

    void init();
    void submit_task_graph();
    void cleanup();

    void startFrame(Camera& camera);
    void endFrame();
};