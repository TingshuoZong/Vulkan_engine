#pragma once

#include <queue>
#include <unordered_set>
#include <variant>

#include "Core/Transform/Transform_messages.h"

#include "Renderer/Meshes/ManagedMesh.h"

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

    void processMessages() {
        //if (!unprocessedMessages) return;
        while (!transformMessages.empty()) {
            const auto& message = transformMessages.front();
            std::visit(
                [&](auto&& TransformUpdatedMessage) {
                    auto meshComponent = meshComponentManager.getComponent(TransformUpdatedMessage.entity_id);
                    auto sharedMeshPointer = meshComponent->mesh.lock();

                    meshComponent->getInstanceData().model_matrix = TransformUpdatedMessage.model_matrix;
                    if (meshComponent && !meshesToUpdate.contains(sharedMeshPointer)) {
                        meshesToUpdate.insert(sharedMeshPointer);
                    }
                }
            , message);
            transformMessages.pop();
        }
    }

    inline void updatePerInstanceData() {
        //if (meshesToUpdate.empty()) return;
        for (auto& mesh : meshesToUpdate) {
            //std::cerr << "Need to reimplement accessing per-instance data\n";
            auto* ptr = device.buffer_host_address_as<PerInstanceData>(renderer.drawGroups[mesh->drawGroupIndex].instance_buffer_id).value();
            memcpy(ptr + mesh->instance_offset, mesh->instance_data.data(), mesh->instance_data.size() * sizeof(PerInstanceData));

            //auto* ptr = device.buffer_host_address_as<PerInstanceData>(mesh->instance_buffer_id).value();
            //memcpy(ptr, mesh->instance_data.data(), mesh->instance_data.size() * sizeof(PerInstanceData));
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
