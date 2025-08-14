#pragma once

#include "shared.inl"  // For UniformBufferObject or PushConstant

#include <daxa/daxa.hpp>
#include <daxa/utils/task_graph.hpp>

#include "Tools/Model_loader.h"


constexpr size_t MAX_INSTANCE_COUNT = 1024;

struct DrawableMesh {
    daxa::Device* device = nullptr;

    std::uint32_t vertex_offset;
    std::uint32_t index_offset;
    std::uint32_t instance_offset;

    std::vector<Vertex> verticies;
    std::vector<uint32_t> indicies;

    std::vector<std::uint32_t> instance_data_offsets;

    size_t drawGroupIndex;

    // ----------------------------------------------------------------
    //daxa::BufferId vertex_buffer_id;
    //daxa::BufferId index_buffer_id;
    //daxa::TaskBuffer task_vertex_buffer;
    //daxa::TaskBuffer task_index_buffer;
    
    
    //daxa::BufferId instance_buffer_id;
    //daxa::TaskBuffer task_instance_buffer;
    // -----------------------------------------------------------------

    std::uint32_t vertex_count;
    std::uint32_t index_count;

    std::vector<PerInstanceData> instance_data;

    std::string name;

    DrawableMesh(ParsedPrimitive parsedPrimitive, std::string name)
        :name(name) {
        vertex_count = parsedPrimitive.vertexCount;
        index_count = parsedPrimitive.indexCount;

        verticies = std::move(parsedPrimitive.vertices);
        indicies = std::move(parsedPrimitive.indices);
    }

    /*DrawableMesh(daxa::Device& device, size_t vertex_count, size_t index_count, std::string name)
        : device(&device), vertex_count(vertex_count), index_count(index_count), name(name) {

        vertex_buffer_id = device.create_buffer({
            .size = sizeof(Vertex) * vertex_count,
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
            .name = name + " task instance SSBO"
        });
    }*/

    /*void cleanup() {
		device->destroy_buffer(vertex_buffer_id);
		device->destroy_buffer(index_buffer_id);
		device->destroy_buffer(instance_buffer_id);
    }*/

    // void upload_mesh_data_task(
    //     daxa::TaskGraph& tg,
    //     const std::vector<Vertex>& vertex_data,
    //     const std::vector<uint32_t>& index_data
    // );
};
