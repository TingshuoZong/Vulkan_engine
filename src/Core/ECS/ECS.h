#pragma once

#include <cstdint>
#include <iostream>
#include <typeindex>
#include <unordered_map>
#include <vector>

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
}