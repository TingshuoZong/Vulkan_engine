#include "DrawGroup.h"

DrawGroup::DrawGroup(daxa::Device& device, const std::shared_ptr<daxa::RasterPipeline> &pipeline, std::string name)
	: device(device), pipeline(pipeline), name(name) {

}

void DrawGroup::add_mesh_instance(uint32_t mesh_index, PerInstanceData data) {
    meshes[mesh_index].lock()->instance_data.push_back(data);

    auto* ptr = device.buffer_host_address_as<PerInstanceData>(meshes[mesh_index].lock()->instance_buffer_id).value();
    memcpy(ptr, meshes[mesh_index].lock()->instance_data.data(), meshes[mesh_index].lock()->instance_data.size() * sizeof(PerInstanceData));
}

void DrawGroup::cleanup() {
	for (auto& mesh : meshes) {
		mesh.lock()->cleanup();
	}
}