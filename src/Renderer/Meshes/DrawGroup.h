#pragma once

#include "shared.inl"
#include "DrawableMesh.h"

#include <daxa/daxa.hpp>
#include <daxa/utils/pipeline_manager.hpp>
#include <daxa/utils/task_graph.hpp>

constexpr size_t MAX_DRAWGROUP_INSTANCE_COUNT = 1024;

struct DrawGroup {
	std::string name;
	size_t drawGroupIndex;

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

	uint32_t total_vertex_count = 0;
	uint32_t total_index_count = 0;

	DrawGroup(daxa::Device& device, const std::shared_ptr<daxa::RasterPipeline> &pipeline, std::string name) 
		:device(device), pipeline(pipeline), name(name) {};
	void cleanup();

	void allocBuffers();
	void reallocBuffers();

	void loadBufferInfo(
		std::vector<Vertex>& vertexStagingArr,
		std::vector<uint32_t>& indexStagingArr,
		std::vector<PerInstanceData>& instanceStagingArr);

	void uploadBufferData(
		daxa::TaskGraph& tg,
		std::vector<Vertex>& vertexStagingArr,
		std::vector<uint32_t>& indexStagingArr,
		std::vector<PerInstanceData>& instanceStagingArr);

	inline void uploadBuffers(daxa::TaskGraph& tg) {
		std::vector<Vertex> vertexStagingArr;
		std::vector<uint32_t> indexStagingArr;
		std::vector<PerInstanceData> instanceStagingArr;

		loadBufferInfo(vertexStagingArr, indexStagingArr, instanceStagingArr);
		allocBuffers();

		tg.use_persistent_buffer(task_vertex_buffer);
		tg.use_persistent_buffer(task_index_buffer);
		tg.use_persistent_buffer(task_instance_buffer);

		uploadBufferData(tg, vertexStagingArr, indexStagingArr, instanceStagingArr);
	}

	inline void register_mesh(std::weak_ptr<DrawableMesh> drawableMesh, daxa::TaskGraph loop_task_graph);

//	void add_mesh_instance(uint32_t mesh_index, PerInstanceData data);

//	inline void use_in_loop_task_graph(uint32_t mesh_index, daxa::TaskGraph& loop_task_graph) {
//	    loop_task_graph.use_persistent_buffer(meshes[mesh_index].lock()->task_vertex_buffer);
//	    loop_task_graph.use_persistent_buffer(meshes[mesh_index].lock()->task_index_buffer);
//        loop_task_graph.use_persistent_buffer(meshes[mesh_index].lock()->task_instance_buffer);
//    };

private:

};



inline void DrawGroup::register_mesh(std::weak_ptr<DrawableMesh> drawableMesh, daxa::TaskGraph loop_task_graph) {
	meshes.push_back(drawableMesh);
	meshes.back().lock()->drawGroupIndex = drawGroupIndex;
	//use_in_loop_task_graph(meshes.size() - 1, loop_task_graph);
}
