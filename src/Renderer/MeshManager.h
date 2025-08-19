#pragma once

#include "Meshes/DrawableMesh.h"
#include "Tools/Model_loader.h"

class MeshManager {
public:
    daxa::Device& device;
    daxa::TaskGraph upload_task_graph;

    std::vector<std::shared_ptr<DrawableMesh>> meshes;

    int meshIndex = -1;

    explicit MeshManager(daxa::Device& device);

    std::weak_ptr<DrawableMesh> add_mesh(const std::string& name, size_t VertexCount, size_t IndexCount);
    std::weak_ptr<DrawableMesh> add_mesh(const std::string& name, ParsedPrimitive parsedPrimitive);


    inline std::weak_ptr<DrawableMesh> get_mesh_ptr(const int mesh_index) {
        return meshes[mesh_index];
    }

    inline void submit_upload_task_graph() {
        upload_task_graph.submit({});
        upload_task_graph.complete({});
        upload_task_graph.execute({});
    }
};

inline MeshManager::MeshManager(daxa::Device &device)
    : device(device) {

    upload_task_graph = daxa::TaskGraph({
        .device = device,
        .name = "Upload task graph",
    });
}

inline std::weak_ptr<DrawableMesh> MeshManager::add_mesh(const std::string& name, ParsedPrimitive parsedPrimitive) {
    meshIndex++;
    meshes.push_back(std::make_shared<DrawableMesh>(parsedPrimitive, name));
    return meshes.back();
}