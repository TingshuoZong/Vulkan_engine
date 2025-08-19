#pragma once

#include "shared.inl"
#include "DrawableMesh.h"

#include <daxa/daxa.hpp>
#include <daxa/utils/pipeline_manager.hpp>
#include <daxa/utils/task_graph.hpp>

#include <vulkan/vulkan.h>

/// @note @c MAX_DRAWGROUP_INSTANCE_COUNT and @c MAX_DRAWGROUP_MESH_COUNT determine the size of the @c task_instance_buffer and the @c task_command_buffer respectively
constexpr size_t MAX_DRAWGROUP_INSTANCE_COUNT = 1024;
constexpr size_t MAX_DRAWGROUP_MESH_COUNT = 1024;

/**
 * @brief DrawGroups act as low-level abstractions to help with aggrgating buffers and indirect rendering
 * 
 * Each buffer owns a @c daxa::RasterPipeline this is the main determiner in whether to put a mesh into a @c DrawGroup
 * DrawGroups also store references to the aggrgate task buffers and buffer ids for the verticies, indicies, instances and indirect draw commands, the actual offsets are stored in @DrawableMesh
 * The buffers except for the instance and command buffers have a fixed size so to load data after you already called @c uploadBuffers you need to call @c reuploadBuffers
 * 
 * @note reuploadBuffers, reallocBuffers (an internal function) have not been implemented yet and the ability to add more instances on the go as well as defragment the instance buffers also need to be added
 * 
 */
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
	daxa::BufferId command_buffer_id;

	daxa::TaskBuffer task_vertex_buffer;
	daxa::TaskBuffer task_index_buffer;
	daxa::TaskBuffer task_instance_buffer;
	daxa::TaskBuffer task_command_buffer;

	/// @brief @c indirectCommands is stored in @c DrawGroup and not the other buffers because @c indirectCommands is the actual @c VkDrawIndexedIndirectCommands
	std::vector<VkDrawIndexedIndirectCommand> indirectCommands;

	uint32_t total_vertex_count = 0;
	uint32_t total_index_count = 0;

	DrawGroup(daxa::Device& device, const std::shared_ptr<daxa::RasterPipeline> &pipeline, std::string name) 
		:device(device), pipeline(pipeline), name(name) {};
	void cleanup();

	/// @brief An internal function that actually allocates the gpu side buffers based off of the internal @c total_vertex_count and @c total_index_count so those need to be set first
	void allocBuffers();
	/// @brief An internal function that reallocates the gpu side buffers based off of the internal @c total_vertex_count and @c total_index_count so those need to be set first
	void reallocBuffers();

	/// @brief An internal function that builds the @c vertexStagingArr, @c indexStagingArr, @c instanceStagingArr and @c indicrectCommands as well as sets the @c total_vertex_count and @c total_index_count
	void loadBufferInfo(
		std::vector<meshRenderer::Vertex>& vertexStagingArr,
		std::vector<uint32_t>& indexStagingArr,
		std::vector<meshRenderer::PerInstanceData>& instanceStagingArr);

	/// @brief An internal function that uploads @c vertexStagingArr, @c indexStagingArr, @c instanceStagingArr and @c indicrectCommands to the GPU
	/// @param tg The @c daxa::TaskGraph that gets assigned the mesh upload tasks
	void uploadBufferData(
		daxa::TaskGraph& tg,
		std::vector<meshRenderer::Vertex>& vertexStagingArr,
		std::vector<uint32_t>& indexStagingArr,
		std::vector<meshRenderer::PerInstanceData>& instanceStagingArr);

	/// @brief Loads the cached data in all of the the @ref DrawableMesh "DrawableMeshes" stored in @c DrawGroup.meshes @c std::vector (calls @c allocBuffers, @c loadBufferInfo and @c uploadBufferData insternally)
	/// @param tg The @c daxa::TaskGraph that gets assigned the mesh upload tasks
	inline void uploadBuffers(daxa::TaskGraph& tg) {
 		std::vector<meshRenderer::Vertex> vertexStagingArr;
		std::vector<uint32_t> indexStagingArr;
		std::vector<meshRenderer::PerInstanceData> instanceStagingArr;

		loadBufferInfo(vertexStagingArr, indexStagingArr, instanceStagingArr);
		allocBuffers();

		tg.use_persistent_buffer(task_vertex_buffer);
		tg.use_persistent_buffer(task_index_buffer);
		tg.use_persistent_buffer(task_command_buffer);
		tg.use_persistent_buffer(task_instance_buffer);

		uploadBufferData(tg, vertexStagingArr, indexStagingArr, instanceStagingArr);
	}

	inline void register_mesh(std::weak_ptr<DrawableMesh> drawableMesh, daxa::TaskGraph loop_task_graph);

private:

};

inline void DrawGroup::register_mesh(std::weak_ptr<DrawableMesh> drawableMesh, daxa::TaskGraph loop_task_graph) {
	meshes.push_back(drawableMesh);
	meshes.back().lock()->drawGroupIndex = drawGroupIndex;
}
