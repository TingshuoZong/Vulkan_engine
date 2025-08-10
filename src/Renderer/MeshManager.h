#pragma once

#include "Meshes/DrawableMesh.h"

class MeshManager {
public:
    daxa::Device& device;
    daxa::TaskGraph upload_task_graph;

    int meshIndex = -1;

    explicit MeshManager(daxa::Device& device);

    std::weak_ptr<DrawableMesh> add_mesh(const std::string& name, size_t VertexCount, size_t IndexCount);


    inline std::weak_ptr<DrawableMesh> get_mesh_ptr(const int mesh_index) {
        return meshes[mesh_index];
    }

    inline void upload_mesh_data_task(
        daxa::TaskGraph& tg,
        const std::vector<Vertex>& vertex_data,
        const std::vector<uint32_t>& index_data) const {
        meshes[meshIndex]->upload_mesh_data_task(tg, vertex_data, index_data);
    }

    inline void submit_upload_task_graph() {
        upload_task_graph.submit({});
        upload_task_graph.complete({});
        upload_task_graph.execute({});
    }

    std::vector<std::shared_ptr<DrawableMesh>> meshes;
};

inline MeshManager::MeshManager(daxa::Device &device)
    : device(device) {

    upload_task_graph = daxa::TaskGraph({
        .device = device,
        .name = "Upload task graph",
    });
}

inline std::weak_ptr<DrawableMesh> MeshManager::add_mesh(const std::string& name, size_t VertexCount, size_t IndexCount) {
    meshIndex++;
    meshes.push_back(std::make_shared<DrawableMesh>(device, VertexCount, IndexCount, name));
    return meshes.back();
}