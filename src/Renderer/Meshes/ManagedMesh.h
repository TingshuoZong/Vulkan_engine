#pragma once

#include "DrawableMesh.h"
#include "DrawGroup.h"
#include "shared.inl"

#include "Core/ECS/ECS_Component.h"
#include "Renderer/MeshManager.h"
#include "Renderer/Renderer.h"

#include "Tools/Model_loader.h"

struct TextureData {
    daxa::ImageViewId albedo;
    daxa_SamplerId tex_sampler{};
};

class ManagedMesh : public IComponent {
public:
    std::weak_ptr<DrawableMesh> mesh;

    ManagedMesh(GLTF_Loader& loader, MeshManager& meshManager, DrawGroup& drawGroup, const Renderer& renderer, TextureData texture)
        : transform(), textures() {
        // Add a new mesh
        mesh = meshManager.add_mesh(loader.path, loader.getModelData(0).vertexCount, loader.getModelData(0).indexCount);
        meshManager.upload_mesh_data_task(0, meshManager.upload_task_graph, loader.getModelData(0).vertices,
                                          loader.getModelData(0).indices);

        drawGroup.register_mesh(meshManager.get_mesh_ptr(0), renderer.loop_task_graph);
        meshManager.submit_upload_task_graph();

        mesh.lock()->instance_data.push_back({
            .model_matrix = glm::mat4(1.0f),
            .texture = texture.albedo,
            .tex_sampler = texture.tex_sampler
        });
    }

    ManagedMesh(const int instanceNo, const std::weak_ptr<DrawableMesh>& mesh, TextureData texture)
        : instanceNo(instanceNo), mesh(mesh), transform(), textures() {
        // Add a instanced mesh

        mesh.lock()->instance_data.push_back({
            .model_matrix = glm::mat4(1.0f),
            .texture = texture.albedo,
            .tex_sampler = texture.tex_sampler
        });
    }

    void instantiate(Entity entity, ComponentManager<ManagedMesh>& componentManager, TextureData textures) {
        if (instanceNo == 0) {
            int nextInstanceNo = numberOfInstances++;
            componentManager.addComponent(entity, ManagedMesh(nextInstanceNo, mesh.lock(), textures));
        } else std::cerr << "Warning: instantiate failed: only the first instance of a mesh can instantiate meshes\n";
    }

    [[nodiscard]] PerInstanceData& getInstanceData() const {
        return mesh.lock()->instance_data[instanceNo];
    }

private:
    int instanceNo = 0;    // If instance is 0 it is the actual mesh that is being instanced
    int numberOfInstances = 1;

    glm::mat4 transform;
    TextureData textures;
};