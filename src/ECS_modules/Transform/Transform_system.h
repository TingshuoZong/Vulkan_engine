#pragma once

#include <queue>
#include <unordered_set>
#include <variant>

#include "ECS_modules/Transform/Transform_messages.h"

#include "ECS_modules/Managed_mesh/ManagedMesh.h"

class TransformSystem : public ISystem {
public:
    TransformSystem(daxa::Device& device, Renderer& renderer, ComponentManager<ManagedMesh>& meshComponentManager)
        : device(device), renderer(renderer), meshComponentManager(meshComponentManager) {}

    inline void update() {
        processMessages();
        updatePerInstanceData();
    }

   inline void enqueueMessage(const TransformMessage& message) {
        unprocessedMessages = true;
        transformMessages.push(message);
    }

    void processMessages() {;
        while (!transformMessages.empty()) {
            const auto& message = transformMessages.front();
            std::visit(
                [&](auto&& TransformUpdatedMessage) {
                    auto meshComponent = meshComponentManager.getComponent(TransformUpdatedMessage.entity_id);
                    auto sharedMeshPointer = meshComponent->mesh.lock();

                    meshComponent->getInstanceData().model_matrix = to_daxa(TransformUpdatedMessage.model_matrix);
                    if (meshComponent && !meshesToUpdate.contains(sharedMeshPointer)) {
                        meshesToUpdate.insert(sharedMeshPointer);
                    }
                }
            , message);
            transformMessages.pop();
        }
    }

    inline void updatePerInstanceData() {
        for (auto& mesh : meshesToUpdate) {
            auto* ptr = device.buffer_host_address_as<meshRenderer::PerInstanceData>(renderer.drawGroups[mesh->drawGroupIndex].instance_buffer_id).value();
            memcpy(ptr + mesh->instance_offset, mesh->instance_data.data(), mesh->instance_data.size() * sizeof(meshRenderer::PerInstanceData));
        }
        meshesToUpdate.clear();
    }
private:
    bool unprocessedMessages = false;

    daxa::Device& device;
    Renderer& renderer;

    ComponentManager<ManagedMesh>& meshComponentManager;

    std::queue<TransformMessage> transformMessages;
    std::unordered_set<std::shared_ptr<DrawableMesh>> meshesToUpdate;
};
