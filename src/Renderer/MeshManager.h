#pragma once

#include "Meshes/DrawableMesh.h"
#include "Tools/Model_loader.h"

/**
 * @brief This is desinged to hold all the @ref DrawableMesh "DrawableMeshes" and the upload @c daxa::TaskGraph as a singleton
 *
 * All @ref DrawableMesh "DrawableMeshes" would typically store in a single @c MeshManager regardless of which drawGroup it is in, this is meant to help with higher-level abstractions such as @ref ManagedMesh \n
 * @c MeshManager is the thing that is actually meant to be the low-level thing that actually makes new @ref DrawableMesh "DrawableMeshes" it is also what the high-level functions call when making a new mesh
 *
 */

class MeshManager {
public:
    daxa::Device& device;
    daxa::TaskGraph upload_task_graph;

    std::vector<std::shared_ptr<DrawableMesh>> meshes;

    int meshIndex = -1;

    explicit MeshManager(daxa::Device& device);

    std::weak_ptr<DrawableMesh> add_mesh(const std::string& name, size_t VertexCount, size_t IndexCount);
    std::weak_ptr<DrawableMesh> add_mesh(const std::string& name, ParsedPrimitive&& parsedPrimitive);

    /// @brief Just returns a @c std::weak_ptr<DrawableMesh> to the mesh based on its index inside the manager
    /// @return A @c weak_ptr to the @ref DrawableMesh
    inline std::weak_ptr<DrawableMesh> get_mesh_ptr(const int mesh_index) {
        return meshes[mesh_index];
    }

    /// @brief Submits @c MeshManager.upload_task_graph with @c submit, @c complete and execute
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

/**
 * @brief Adds a mesh to the mesh manager with a rvalue reference to a parsedPrimitive
 *
 * @param name The internal name of the mesh, it will be combined with other names and strings for the names of its buffers etc.
 * @param parsedPrimitive A rvalue reference to the parsedPrimitive you want to add
 *
 * @return A weak pointer to the @c std::shared_ptr<DrawableMesh> that was created
 */
inline std::weak_ptr<DrawableMesh> MeshManager::add_mesh(const std::string& name, ParsedPrimitive&& parsedPrimitive) {
    meshIndex++;
    meshes.push_back(std::make_shared<DrawableMesh>(parsedPrimitive, name));
    return meshes.back();
}