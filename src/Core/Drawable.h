#pragma once

#include "shared.inl"  // For UniformBufferObject or MyPushConstant

#include <daxa/daxa.hpp>
#include <daxa/utils/task_graph.hpp>

constexpr size_t MAX_INSTANCE_COUNT = 1024;

struct Drawable {
    daxa::BufferId vertex_buffer_id;
    daxa::BufferId index_buffer_id;
    daxa::TaskBuffer task_vertex_buffer;
    daxa::TaskBuffer task_index_buffer;

    uint32_t vertex_count;
    uint32_t index_count;

    daxa::BufferId instance_buffer_id;
    daxa::TaskBuffer task_instance_buffer;
    std::vector<PerInstanceData> instance_data;

    std::string name;

    Drawable(daxa::Device& device, daxa::TaskGraph upload_task_graph, uint32_t vertex_count, uint32_t index_count, std::string name)
        : vertex_count(vertex_count), index_count(index_count), name(name) {

        vertex_buffer_id = device.create_buffer({
            .size = sizeof(MyVertex) * vertex_count,
            .name = name + " vertex buffer",
        });
        index_buffer_id = device.create_buffer({
            .size = sizeof(uint32_t) * index_count,
            .name = name + " index buffer",
        });

        task_vertex_buffer = daxa::TaskBuffer({
            .initial_buffers = {.buffers = std::span{&vertex_buffer_id, 1}},
            .name = name + " task vertex buffer",
        });
        task_index_buffer = daxa::TaskBuffer({
            .initial_buffers = {.buffers = std::span{&index_buffer_id, 1}},
            .name = name + "task index buffer",
        });

        instance_buffer_id = device.create_buffer({
            .size = sizeof(PerInstanceData) * MAX_INSTANCE_COUNT,
            .allocate_info = daxa::MemoryFlagBits::HOST_ACCESS_RANDOM,
            .name = name + " instance SSBO",
        });
        task_instance_buffer = daxa::TaskBuffer({
            .initial_buffers = {.buffers = std::span{&instance_buffer_id, 1} },
            .name = name + " instance buffer"
        });
    }
};