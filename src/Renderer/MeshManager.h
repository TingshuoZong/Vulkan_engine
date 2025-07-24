#pragma once

#include "DrawableMesh.h"

class MeshManager {
public:
    daxa::Device& device;

    MeshManager(daxa::Device& device);

    template<size_t VertexCount, size_t IndexCount>
    void add_mesh(std::string name);


    inline std::weak_ptr<DrawableMesh> get_mesh_ptr(int mesh_index) {
        return meshes[mesh_index];
    }
public:
    std::vector<std::shared_ptr<DrawableMesh>> meshes;
};

inline MeshManager::MeshManager(daxa::Device &device)
    : device(device) {}

template<size_t VertexCount, size_t IndexCount>
void MeshManager::add_mesh(std::string name) {
    meshes.push_back(std::make_shared<DrawableMesh>(device, VertexCount, IndexCount, name));
}