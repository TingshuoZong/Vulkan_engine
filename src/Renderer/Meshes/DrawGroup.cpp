#include "DrawGroup.h"

//void DrawGroup::add_mesh_instance(uint32_t mesh_index, PerInstanceData data) {
//    meshes[mesh_index].lock()->instance_data.push_back(data);
//
//    auto* ptr = device.buffer_host_address_as<PerInstanceData>(meshes[mesh_index].lock()->instance_buffer_id).value();
//    memcpy(ptr, meshes[mesh_index].lock()->instance_data.data(), meshes[mesh_index].lock()->instance_data.size() * sizeof(PerInstanceData));
//}

void DrawGroup::cleanup() {
     device.destroy(vertex_buffer_id);
     device.destroy(index_buffer_id);
     device.destroy(instance_buffer_id);

    /*for (auto& mesh : meshes) {
		mesh.lock()->cleanup();
    }*/
}

void DrawGroup::allocBuffers() {
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
		.size = MAX_DRAWGROUP_INSTANCE_COUNT * sizeof(PerInstanceData),
		.allocate_info = daxa::MemoryFlagBits::HOST_ACCESS_RANDOM,
		.name = name + " instance SSBO"
		});

	task_instance_buffer = daxa::TaskBuffer({
		.initial_buffers = {.buffers = std::span{&instance_buffer_id, 1}},
		.name = name + " task instance SSBO"
		});
}

void DrawGroup::loadBufferInfo(
	std::vector<Vertex>& vertexStagingArr,
	std::vector<uint32_t>& indexStagingArr,
	std::vector<PerInstanceData>& instanceStagingArr)
{
	// Create big buffers
	uint32_t currentIndexCount = 0;
	uint32_t currentVertexCount = 0;
	uint32_t currentInstanceCount = 0;

	for (auto& mesh : meshes) {
		std::shared_ptr<DrawableMesh> meshPtr = mesh.lock();

		total_vertex_count += meshPtr->vertex_count;
		total_index_count += meshPtr->index_count;
	}

	vertexStagingArr.reserve(total_vertex_count);
	indexStagingArr.reserve(total_index_count);
	instanceStagingArr.reserve(MAX_DRAWGROUP_INSTANCE_COUNT);

	for (auto& mesh : meshes) {
		std::shared_ptr<DrawableMesh> meshPtr = mesh.lock();

		meshPtr->vertex_offset = currentVertexCount;
		meshPtr->index_offset = currentIndexCount;
		meshPtr->instance_offset = currentInstanceCount;

	    currentVertexCount += meshPtr->vertex_count;
	    currentIndexCount += meshPtr->index_count;

		meshPtr->instance_data_offsets.reserve(meshPtr->instance_data.size());
		for (int i = 0; i < meshPtr->instance_data.size(); i++) {
			meshPtr->instance_data_offsets.push_back(currentInstanceCount);
			currentInstanceCount++;
		}

		vertexStagingArr.insert(vertexStagingArr.end(), meshPtr->verticies.begin(), meshPtr->verticies.end());
		indexStagingArr.insert(indexStagingArr.end(), meshPtr->indicies.begin(), meshPtr->indicies.end());
		instanceStagingArr.insert(instanceStagingArr.end(), meshPtr->instance_data.begin(), meshPtr->instance_data.end());
	}

	if (currentInstanceCount > MAX_DRAWGROUP_INSTANCE_COUNT)
	   throw std::runtime_error("Error: DrawGroup instance count exceeded, either bind less instances to the drawgroup or increase MAX_DRAWGROUP_INSTANCE_COUNT");
}

void DrawGroup::uploadBufferData(
	daxa::TaskGraph& tg, 
	std::vector<Vertex>& vertexStagingArr, 
	std::vector<uint32_t>& indexStagingArr, 
	std::vector<PerInstanceData>& instanceStagingArr) 
{
	tg.add_task({
		.attachments = {
			daxa::inl_attachment(daxa::TaskBufferAccess::TRANSFER_WRITE, this->task_vertex_buffer),
			daxa::inl_attachment(daxa::TaskBufferAccess::TRANSFER_WRITE, this->task_index_buffer),
		},
		.task = [=, this](daxa::TaskInterface ti) {
			auto vertex_staging = ti.device.create_buffer({
						   .size = vertexStagingArr.size() * sizeof(Vertex),
						   .allocate_info = daxa::MemoryFlagBits::HOST_ACCESS_RANDOM,
						   .name = this->name + ">" + name + "vertex staging buffer",
			});
			ti.recorder.destroy_buffer_deferred(vertex_staging);
			auto* vertex_ptr = ti.device.buffer_host_address_as<Vertex>(vertex_staging).value();
			memcpy(vertex_ptr, vertexStagingArr.data(), vertexStagingArr.size() * sizeof(Vertex));

			ti.recorder.copy_buffer_to_buffer({
				.src_buffer = vertex_staging,
				.dst_buffer = ti.get(this->task_vertex_buffer).ids[0],
				.size = vertexStagingArr.size() * sizeof(Vertex),
			});

			auto index_staging = ti.device.create_buffer({
				.size = indexStagingArr.size() * sizeof(uint32_t),
				.allocate_info = daxa::MemoryFlagBits::HOST_ACCESS_RANDOM,
				.name = this->name + ">" + name + " index staging buffer",
				});
			ti.recorder.destroy_buffer_deferred(index_staging);
			auto* index_ptr = ti.device.buffer_host_address_as<uint32_t>(index_staging).value();
			std::memcpy(index_ptr, indexStagingArr.data(), indexStagingArr.size() * sizeof(uint32_t));

			ti.recorder.copy_buffer_to_buffer({
				.src_buffer = index_staging,
				.dst_buffer = ti.get(this->task_index_buffer).ids[0],
				.size = indexStagingArr.size() * sizeof(uint32_t),
			});
		},
		.name = this->name + ">" + name + " upload mesh data",
	});
}