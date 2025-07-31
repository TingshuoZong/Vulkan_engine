#pragma once

#include <cstdint>
#include <iostream>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include "ECS_Component.h"

static Entity next_entity_id = 1;

class EntityManager {
public:
    static Entity createEntity() {
        next_entity_id++;
        return next_entity_id - 1;
    }

    template<typename T>
    ComponentManager<T>& getComponentManager() {
        const auto typeIndex = std::type_index(typeid(T));
        const auto componentManager = componentManagers.find(typeIndex);

        if (componentManager == componentManagers.end()) {
            auto manager = std::make_unique<ComponentManager<T>>();

            ComponentManager<T>* ptr = manager.get();
            componentManagers[typeIndex] = std::move(manager);

            return *ptr;
        }
        return *static_cast<ComponentManager<T>*>(componentManager->second.get());
    }
private:
    std::unordered_map<std::type_index, std::unique_ptr<IComponentManager>> componentManagers;
};