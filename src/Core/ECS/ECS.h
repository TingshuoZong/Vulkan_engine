#pragma once

#include "ECS_Entity.h"
#include "ECS_Component.h"

namespace ecs {
    inline EntityManager entityManager;

    template <typename T>
    inline T* getComponent(Entity entity) {
        return entityManager.getComponentManager<T>().getComponent(entity);
    }

    template <typename T>
    inline ComponentManager<T>& getComponentManager() {
        return entityManager.getComponentManager<T>();
    }

    template <typename T>
    inline T* getSystem() {
        return entityManager.getSystemManager().getSystem();
    }

    inline SystemManager& getSystemManager() {
        return entityManager.getSystemManager();
    }

    inline void updateSystems() {
        entityManager.updateSystems();
    }
}