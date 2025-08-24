#include "Transform_component.h"
#include "core/ECS/ECS_Entity.h"

#include <glm/ext/matrix_transform.hpp>

TransformComponent::TransformComponent(EntityManager& entityManager, glm::vec3 position, glm::vec3 eulerRotation, glm::vec3 scale)
        :entityManager(std::ref(entityManager)),
         transformSystem(std::ref(entityManager.getSystem<TransformSystem>())),
         transformComponentManager(std::ref(entityManager.getComponentManager<TransformComponent>())),
         position(position),
         eulerRotation(eulerRotation),
         scale(scale) {
    quaternionRotation  = glm::yawPitchRoll(eulerRotation.y, eulerRotation.x, eulerRotation.z);
    glm::mat4 rotMat = glm::toMat4(quaternionRotation);
    glm::mat4 transMat = glm::translate(glm::mat4(1.0f), position);
    glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), scale);

    modelMatrix = transMat * rotMat * scaleMat;
}

TransformComponent::TransformComponent(EntityManager& entityManager, glm::vec3 position, glm::quat quaternionRotation, glm::vec3 scale)
        :entityManager(std::ref(entityManager)),
         transformSystem(std::ref(entityManager.getSystem<TransformSystem>())),
         transformComponentManager(std::ref(entityManager.getComponentManager<TransformComponent>())),
         position(),
         quaternionRotation(quaternionRotation),
         scale(scale) {
    eulerRotation = glm::eulerAngles(quaternionRotation);
    glm::mat4 rotMat = glm::toMat4(quaternionRotation);
    glm::mat4 transMat = glm::translate(glm::mat4(1.0f), position);
    glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), scale);

    modelMatrix = transMat * rotMat * scaleMat;
}

void TransformComponent::initialize(Entity initializedEntity) {
    thisEntity = initializedEntity;
    transformSystem.get().enqueueMessage(TransformUpdatedMessage{
        .entity_id = thisEntity,
        .model_matrix = modelMatrix,
    });
}

void TransformComponent::updateModelMatrix() {
    if (quaternionDirty) {
        quaternionRotation  = glm::yawPitchRoll(eulerRotation.y, eulerRotation.x, eulerRotation.z);
        quaternionDirty = false;
    }

    glm::mat4 rotMat = glm::toMat4(quaternionRotation);
    glm::mat4 transMat = glm::translate(glm::mat4(1.0f), position);
    glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), scale);

    modelMatrix = transMat * rotMat * scaleMat;

    transformSystem.get().enqueueMessage(TransformUpdatedMessage{
        .entity_id = thisEntity,
        .model_matrix = modelMatrix,
    });
}
