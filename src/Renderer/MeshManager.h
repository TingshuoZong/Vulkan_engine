#pragma once

#include "DrawableMesh.h"

class MeshManager {
public:
    daxa::Device& device;
    daxa::TaskGraph upload_task_graph;

    MeshManager(daxa::Device& device);

    void add_mesh(std::string name, size_t VertexCount, size_t IndexCount);


    inline std::weak_ptr<DrawableMesh> get_mesh_ptr(int mesh_index) {
        return meshes[mesh_index];
    }

    inline void upload_mesh_data_task(
        int mesh_index,
        daxa::TaskGraph& tg,
        const std::vector<Vertex>& vertex_data,
        const std::vector<uint32_t>& index_data) {
        meshes[mesh_index]->upload_mesh_data_task(tg, vertex_data, index_data);
    }

    inline void submit_upload_task_graph() {
        upload_task_graph.submit({});
        upload_task_graph.complete({});
        upload_task_graph.execute({});
    }
public:
    std::vector<std::shared_ptr<DrawableMesh>> meshes;
};

inline MeshManager::MeshManager(daxa::Device &device)
    : device(device) {

    upload_task_graph = daxa::TaskGraph({
        .device = device,
        .name = "Upload task graph",
    });
}

void MeshManager::add_mesh(std::string name, size_t VertexCount, size_t IndexCount) {
    meshes.push_back(std::make_shared<DrawableMesh>(device, VertexCount, IndexCount, name));
}