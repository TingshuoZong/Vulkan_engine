#pragma once

#include <iostream>
#include <typeindex>
#include <unordered_map>

#include "ECS_Component.h"
#include "ECS_System.h"

static Entity next_entity_id = 1;

class EntityManager {
public:
    static Entity createEntity() {
        next_entity_id++;
        return next_entity_id - 1;
    }

    template <typename T>
    void registerComponentManager() {
        const auto typeIndex = std::type_index(typeid(T));
        if (!componentManagers.contains(typeIndex)) {
            auto manager = std::make_unique<ComponentManager<T>>();
            componentManagers[typeIndex] = std::move(manager);
        }
    }

    template <typename T>
    ComponentManager<T>& getComponentManager() {
        const auto typeIndex = std::type_index(typeid(T));
        const auto componentManager = componentManagers.find(typeIndex);

        if (componentManager != componentManagers.end())
            return *static_cast<ComponentManager<T>*>(componentManager->second.get());
        else
            throw std::runtime_error("Error: EntityManager does not have a componentManager");
    }

    template <typename T, typename... Args>
    inline void registerSystem(Args&&... args) {
        systemManager.registerSystem<T>(std::forward<Args>(args)...);
    }

    template <typename T>
    inline T& getSystem() {
        return systemManager.getSystem<T>();
    }

    inline SystemManager& getSystemManager() {
		return systemManager;
	}
    
    inline void updateSystems() {
        systemManager.updateSystems();
    }
private:
    std::unordered_map<std::type_index, std::unique_ptr<IComponentManager>> componentManagers;
    SystemManager systemManager;
};