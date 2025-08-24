#pragma once

#include "Renderer/Meshes/DrawableMesh.h"
#include "Renderer/Meshes/DrawGroup.h"
#include "mesh_rendering_shared.inl"

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

    ManagedMesh(GLTF_Loader& loader, int meshNo, int primitiveNo, MeshManager& meshManager, DrawGroup& drawGroup, const Renderer& renderer, TextureData texture)
        : transform(), textures() {
        mesh = meshManager.add_mesh(loader.path + std::to_string(meshNo) + std::to_string(primitiveNo), loader.getModelData(meshNo, primitiveNo));

        drawGroup.register_mesh(meshManager.get_mesh_ptr(meshManager.meshIndex), renderer.loop_task_graph);

        mesh.lock()->instance_data.push_back({
            .model_matrix = to_daxa(glm::mat4(1.0f)),
            .texture = texture.albedo,
            .tex_sampler = texture.tex_sampler
        });
    }

    ManagedMesh(const int instanceNo, const std::weak_ptr<DrawableMesh>& mesh, TextureData texture)
        : instanceNo(instanceNo), mesh(mesh), transform(), textures() {
        // Add an instanced mesh

        mesh.lock()->instance_data.push_back({
            .model_matrix = to_daxa(glm::mat4(1.0f)),
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

    [[nodiscard]] meshRenderer::PerInstanceData& getInstanceData() const {
        return mesh.lock()->instance_data[instanceNo];
    }

private:
    int instanceNo = 0;    // If instance is 0 it is the actual mesh that is being instanced
    int numberOfInstances = 1;

    glm::mat4 transform;
    TextureData textures;
};