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
    daxa_SamplerId tex_sampler;
};

class ManagedMesh {
public:
    std::weak_ptr<DrawableMesh> mesh;

    ManagedMesh(GLTF_Loader& loader, MeshManager& meshManager, DrawGroup& drawGroup, Renderer renderer, TextureData texture)
        : transform(), textures() {
        meshManager.add_mesh(loader.path, loader.getModelData(0).vertexCount, loader.getModelData(0).indexCount);
        meshManager.upload_mesh_data_task(0, meshManager.upload_task_graph, loader.getModelData(0).vertices,
                                          loader.getModelData(0).indices);

        drawGroup.register_mesh(meshManager.get_mesh_ptr(0), renderer.loop_task_graph);
        meshManager.submit_upload_task_graph();

        mesh = meshManager.get_mesh_ptr(0);

        mesh.lock()->instance_data.push_back({
            .model_matrix = glm::mat4(1.0f),
            .texture = texture.albedo,
            .tex_sampler = texture.tex_sampler
        });
    }

    ManagedMesh(int instanceNo, std::weak_ptr<DrawableMesh> mesh, TextureData texture)
        : instanceNo(instanceNo), mesh(mesh), transform(), textures() {
        mesh.lock()->instance_data.push_back({
            .model_matrix = glm::mat4(1.0f),
            .texture = texture.albedo,
            .tex_sampler = texture.tex_sampler
        });
    }

    void instanciate(Entity entity, ComponentManager<ManagedMesh>& componentManager, TextureData textures) {
        if (instanceNo == 0) {
            componentManager.addComponent(entity, ManagedMesh(numberOfInstances, mesh.lock(), textures));
            numberOfInstances++;
        } else std::cerr << "Warning: Instanciate failed: only the first instance of a mesh can instanciate meshes\n";
    }

    PerInstanceData& getInstanceData() {
        return mesh.lock()->instance_data[instanceNo];
    }

private:
    int instanceNo = 0;    // If instanceo is 0 it is the actual mesh that is being instanced
    int numberOfInstances = 1;

    glm::mat4 transform;
    TextureData textures;
};