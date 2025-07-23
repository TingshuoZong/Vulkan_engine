#pragma once

#include "shared.inl"
#include "DrawableMesh.h"

#include <daxa/daxa.hpp>
#include <daxa/utils/pipeline_manager.hpp>
#include <daxa/utils/task_graph.hpp>

struct DrawGroup {
	std::string name;
	std::vector<DrawableMesh> meshes;
	std::shared_ptr<daxa::RasterPipeline> pipeline;

	daxa::Device& device;

	DrawGroup(daxa::Device& device, const std::shared_ptr<daxa::RasterPipeline> &pipeline, std::string name);
	void cleanup();

	template<size_t VertexCount, size_t IndexCount>
	void add_mesh(const std::array<MyVertex, VertexCount>& vertex_data, const std::array<uint32_t, IndexCount>& index_data, std::string name);
	template<size_t VertexCount, size_t IndexCount>
	void update_mesh(uint32_t mesh_index, daxa::TaskGraph& loop_task_graph, const std::array<MyVertex, VertexCount>& vertex_data, const std::array<uint32_t, IndexCount>& index_data);

	void add_mesh_instance(uint32_t mesh_index, PerInstanceData data);
	
	inline void use_in_loop_task_graph(uint32_t mesh_index, daxa::TaskGraph& loop_task_graph) { 
        loop_task_graph.use_persistent_buffer(meshes[mesh_index].task_vertex_buffer);
        loop_task_graph.use_persistent_buffer(meshes[mesh_index].task_index_buffer);
        loop_task_graph.use_persistent_buffer(meshes[mesh_index].task_instance_buffer);
    };

private:
	template<size_t VertexCount, size_t IndexCount>
	void upload_mesh_data_task(
		daxa::TaskGraph& tg,
		DrawableMesh& drawableMesh,
		const std::array<MyVertex, VertexCount>& vertex_data,
		const std::array<uint32_t, IndexCount>& index_data
	);
};


template<size_t VertexCount, size_t IndexCount>
void DrawGroup::add_mesh(const std::array<MyVertex, VertexCount>& vertex_data, const std::array<uint32_t, IndexCount>& index_data, std::string name) {
	DrawableMesh drawableMesh(device, VertexCount, IndexCount, this->name + ">" + name);

	auto upload_task_graph = daxa::TaskGraph({
		.device = device,
		.name = this->name + ">" + name + " upload",
	});

    upload_task_graph.use_persistent_buffer(drawableMesh.task_vertex_buffer);
    upload_task_graph.use_persistent_buffer(drawableMesh.task_index_buffer);

	upload_mesh_data_task<VertexCount, IndexCount>(upload_task_graph, drawableMesh, vertex_data, index_data);

	upload_task_graph.submit({});
	upload_task_graph.complete({});
	upload_task_graph.execute({});

	meshes.push_back(drawableMesh);
}

template<size_t VertexCount, size_t IndexCount>
void DrawGroup::upload_mesh_data_task(
    daxa::TaskGraph& tg,
    DrawableMesh& drawableMesh,
    const std::array<MyVertex, VertexCount>& vertex_data,
    const std::array<uint32_t, IndexCount>& index_data
) {
    tg.add_task({
        .attachments = {
            daxa::inl_attachment(daxa::TaskBufferAccess::TRANSFER_WRITE, drawableMesh.task_vertex_buffer),
            daxa::inl_attachment(daxa::TaskBufferAccess::TRANSFER_WRITE, drawableMesh.task_index_buffer),
        },
        .task = [=](daxa::TaskInterface ti) {
            auto vertex_staging = ti.device.create_buffer({
                           .size = sizeof(vertex_data),
                           .allocate_info = daxa::MemoryFlagBits::HOST_ACCESS_RANDOM,
                           .name = this->name + ">" + name + "vertex staging buffer",
            });
            ti.recorder.destroy_buffer_deferred(vertex_staging);
            auto* vertex_ptr = ti.device.buffer_host_address_as<std::array<MyVertex, VertexCount>>(vertex_staging).value();
            *vertex_ptr = vertex_data;
            ti.recorder.copy_buffer_to_buffer({
                .src_buffer = vertex_staging,
                .dst_buffer = ti.get(drawableMesh.task_vertex_buffer).ids[0],
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
                .dst_buffer = ti.get(drawableMesh.task_index_buffer).ids[0],
                .size = sizeof(index_data),
            });
        },
        .name = this->name + ">" + name + " upload mesh data",
        });
}
