#pragma once

#include "shared.inl"
#include "DrawableMesh.h"

#include <daxa/daxa.hpp>
#include <daxa/utils/pipeline_manager.hpp>
#include <daxa/utils/task_graph.hpp>

struct DrawGroup {
	std::string name;
	std::vector<std::weak_ptr<DrawableMesh>> meshes;
	std::shared_ptr<daxa::RasterPipeline> pipeline;

	daxa::Device& device;

	DrawGroup(daxa::Device& device, const std::shared_ptr<daxa::RasterPipeline> &pipeline, std::string name);
	void cleanup();

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
