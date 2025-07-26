#pragma once

#include "shared.inl"  // For UniformBufferObject or PushConstant

#include <daxa/daxa.hpp>
#include <daxa/utils/task_graph.hpp>


constexpr size_t MAX_INSTANCE_COUNT = 1024;

struct DrawableMesh {
    daxa::Device* device = nullptr;

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

    DrawableMesh(daxa::Device& device, size_t vertex_count, size_t index_count, std::string name)
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
    }

    void cleanup() {
        device->destroy_buffer(instance_buffer_id);
        device->destroy_buffer(vertex_buffer_id);
        device->destroy_buffer(index_buffer_id);
    }

    template<size_t VertexCount, size_t IndexCount>
    void upload_mesh_data_task(
        daxa::TaskGraph& tg,
        const std::array<Vertex, VertexCount>& vertex_data,
        const std::array<uint32_t, IndexCount>& index_data
    );
};

template<size_t VertexCount, size_t IndexCount>
void DrawableMesh::upload_mesh_data_task(
    daxa::TaskGraph& tg,
    const std::array<Vertex, VertexCount>& vertex_data,
    const std::array<uint32_t, IndexCount>& index_data
) {
    tg.use_persistent_buffer(task_vertex_buffer);
    tg.use_persistent_buffer(task_index_buffer);

    tg.add_task({
        .attachments = {
            daxa::inl_attachment(daxa::TaskBufferAccess::TRANSFER_WRITE, this->task_vertex_buffer),
            daxa::inl_attachment(daxa::TaskBufferAccess::TRANSFER_WRITE, this->task_index_buffer),
        },
        .task = [=](daxa::TaskInterface ti) {
            auto vertex_staging = ti.device.create_buffer({
                           .size = sizeof(vertex_data),
                           .allocate_info = daxa::MemoryFlagBits::HOST_ACCESS_RANDOM,
                           .name = this->name + ">" + name + "vertex staging buffer",
            });
            ti.recorder.destroy_buffer_deferred(vertex_staging);
            auto* vertex_ptr = ti.device.buffer_host_address_as<std::array<Vertex, VertexCount>>(vertex_staging).value();
            *vertex_ptr = vertex_data;
            ti.recorder.copy_buffer_to_buffer({
                .src_buffer = vertex_staging,
                .dst_buffer = ti.get(this->task_vertex_buffer).ids[0],
                .size = sizeof(vertex_data),
            });

            auto index_staging = ti.device.create_buffer({
                .size = sizeof(index_data),
                .allocate_info = daxa::MemoryFlagBits::HOST_ACCESS_RANDOM,
                .name = this->name + ">" + name + " index staging buffer",
                });
            ti.recorder.destroy_buffer_deferred(index_staging);
            auto* index_ptr = ti.device.buffer_host_address_as<std::array<uint32_t, IndexCount>>(index_staging).value();
            *index_ptr = index_data;
            ti.recorder.copy_buffer_to_buffer({
                .src_buffer = index_staging,
                .dst_buffer = ti.get(this->task_index_buffer).ids[0],
                .size = sizeof(index_data),
            });
        },
        .name = this->name + ">" + name + " upload mesh data",
    });
}