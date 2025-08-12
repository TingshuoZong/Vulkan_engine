#pragma once

#include "shared.inl"
#include "DrawableMesh.h"

#include <daxa/daxa.hpp>
#include <daxa/utils/pipeline_manager.hpp>
#include <daxa/utils/task_graph.hpp>

constexpr size_t MAX_DRAWGROUP_INSTANCE_COUNT = 1024;

struct DrawGroup {
	std::string name;
	std::vector<std::weak_ptr<DrawableMesh>> meshes;
	std::shared_ptr<daxa::RasterPipeline> pipeline;

	daxa::Device& device;

	// Big buffers
	daxa::BufferId vertex_buffer_id;
	daxa::BufferId index_buffer_id;
	daxa::BufferId instance_buffer_id;

	daxa::TaskBuffer task_vertex_buffer;
	daxa::TaskBuffer task_index_buffer;
	daxa::TaskBuffer task_instance_buffer;

	uint32_t total_vertex_count;
	uint32_t total_index_count;

	DrawGroup(daxa::Device& device, const std::shared_ptr<daxa::RasterPipeline> &pipeline, std::string name);
	void cleanup();

	/*inline void allocBuffers(daxa::Device& device) {
		vertex_buffer_id = device.create_buffer({
			.size = total_vertex_count * sizeof(Vertex),
			.name = name + " vertex buffer"
		});

		task_vertex_buffer = daxa::TaskBuffer({
			.initial_buffers = {.buffers = std::span{&vertex_buffer_id, 1}},
			.name = name + " task vertex buffer"
		});

		index_buffer_id = device.create_buffer({
				.size = total_index_count * sizeof(uint32_t),
				.name = name + " index buffer"
			});

		task_index_buffer = daxa::TaskBuffer({
			.initial_buffers = {.buffers = std::span{&index_buffer_id, 1}},
			.name = name + " task index buffer"
		});

		instance_buffer_id = device.create_buffer({
			.allocate_info = daxa::MemoryFlagBits::HOST_ACCESS_RANDOM,
			.size = MAX_DRAWGROUP_INSTANCE_COUNT * sizeof(PerInstanceData),
			.name = name + " instance SSBO"
		});

		task_instance_buffer = daxa::TaskBuffer({
			.initial_buffers = {.buffers = std::span{&instance_buffer_id, 1}},
			.name = name + " task instance SSBO"
		});
	}*/

	inline void register_mesh(std::weak_ptr<DrawableMesh> drawableMesh, daxa::TaskGraph loop_task_graph);

	void add_mesh_instance(uint32_t mesh_index, PerInstanceData data);

	inline void use_in_loop_task_graph(uint32_t mesh_index, daxa::TaskGraph& loop_task_graph) {
	    loop_task_graph.use_persistent_buffer(meshes[mesh_index].lock()->task_vertex_buffer);
	    loop_task_graph.use_persistent_buffer(meshes[mesh_index].lock()->task_index_buffer);
            loop_task_graph.use_persistent_buffer(meshes[mesh_index].lock()->task_instance_buffer);
        };

private:

};


inline void DrawGroup::register_mesh(std::weak_ptr<DrawableMesh> drawableMesh, daxa::TaskGraph loop_task_graph) {
	meshes.push_back(drawableMesh);
	use_in_loop_task_graph(meshes.size() - 1, loop_task_graph);
}
