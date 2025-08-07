#pragma once

#include "Core/Transform/Transform_messages.h"
#include "Core/Transform/Transform_system.h"

#include <glm/gtx/euler_angles.hpp>

class TransformComponent : public IComponent {
public:
    TransformComponent(EntityManager& entityManager, glm::vec3 position, glm::vec3 eulerRotation, glm::vec3 scale);
    TransformComponent(EntityManager& entityManager, glm::vec3 position, glm::quat quaternionRotation, glm::vec3 scale);
    void initialize(Entity initializedEntity) override;

    inline void setRotation(glm::vec3 newEulerRotation) {
        eulerRotation = newEulerRotation;
        quaternionDirty = true;
        updateModelMatrix();
    }

    inline void setRotation(glm::quat newQuaternionRotation) {
        quaternionRotation = newQuaternionRotation;
        eulerDirty = true;
        updateModelMatrix();
    }

    inline void setModelMatrix(glm::mat4 newModelMatrix) {
        modelMatrix = newModelMatrix;
		quaternionDirty = true;
		eulerDirty = true;
        updateModelMatrix();
    }

    inline void setPosition(glm::vec3 newPosition) {
        position = newPosition;
        updateModelMatrix();
    }

    inline void setScale(glm::vec3 newSscale) {
        scale = newSscale;
        updateModelMatrix();
    }

    inline glm::vec3 getEulerRotation() {
        if (!eulerDirty)
            return eulerRotation;
        else {
            eulerRotation = glm::eulerAngles(quaternionRotation);
            eulerDirty = false;
            return eulerRotation;
        }
    }
    inline glm::quat getQuaternionRotation() {
        if (!quaternionDirty)
            return quaternionRotation;
        else {
            quaternionRotation  = glm::yawPitchRoll(eulerRotation.y, eulerRotation.x, eulerRotation.z);
            quaternionDirty = false;
            return quaternionRotation;
        }
    }

    inline glm::vec3 getPosition() const {return position;}
    inline glm::vec3 getScale() const {return scale;}

private:
    std::reference_wrapper<EntityManager> entityManager;
    std::reference_wrapper<TransformSystem> transformSystem;
    std::reference_wrapper<ComponentManager<TransformComponent>> transformComponentManager;

    bool eulerDirty = false;
    bool quaternionDirty = false;

    glm::vec3 position;
    glm::vec3 eulerRotation;
    glm::quat quaternionRotation;
    glm::vec3 scale;
    glm::mat4 modelMatrix;

    void updateModelMatrix();
};