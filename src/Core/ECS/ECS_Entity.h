#pragma once

#include <iostream>
#include <typeindex>
#include <unordered_map>

#include "ECS_Component.h"
#include "ECS_System.h"

static Entity next_entity_id = 1;

/**
 * @brief A singleton class that manages the whole ECS accessed through the @c ecs namespace
 * All of the @ref ComponentManager and @ref SystemManager for each type of component and system should be registered with this
 * The @ref ComponentManagers are stored in a @c std::unordered_map based on their types
 */
class EntityManager {
public:
    /// @brief Allocates an ID for a new entity
    /// @return The entityID for the new entity
    static Entity createEntity() {
        next_entity_id++;
        return next_entity_id - 1;
    }

    /// @brief Registers a @ref ComponentManager if one of that type is not already registered
    /// @tparam T The type of the @ref ComponentManager
    template <typename T>
    void registerComponentManager() {
        const auto typeIndex = std::type_index(typeid(T));
        if (!componentManagers.contains(typeIndex)) {
            auto manager = std::make_unique<ComponentManager<T>>();
            componentManagers[typeIndex] = std::move(manager);
        }
    }

    /// @brief Used to get a reference to the @ref ComponentManager
    /// @tparam T The type of the @ref ComponentManager to get a reference for
    /// @return A reference to the @ref ComponentManager
    template <typename T>
    ComponentManager<T>& getComponentManager() {
        const auto typeIndex = std::type_index(typeid(T));
        const auto componentManager = componentManagers.find(typeIndex);

        if (componentManager != componentManagers.end())
            return *static_cast<ComponentManager<T>*>(componentManager->second.get());
        else
            throw std::runtime_error("Error: EntityManager does not have a componentManager");
    }

    /**
     * @brief Forwards registration to the @c SystemManager
     * 
     * This is different to a @ref ComponentManager because the there is only one @c SystemManager as there cannot be multiple instances of a system
     * 
     * @tparam T The type of the system you want to register
     * @tparam Args Arguments forwarded to the constructor of the system
     * @param args Arguments forwarded to the constructor of the system
     */
    template <typename T, typename... Args>
    inline void registerSystem(Args&&... args) {
        systemManager.registerSystem<T>(std::forward<Args>(args)...);
    }

    /// @brief Returns a reference to the system based off of the type of it
    /// @tparam T The type of the system to be returned
    /// @return A reference to the system
    template <typename T>
    inline T& getSystem() {
        return systemManager.getSystem<T>();
    }

    /// @brief Returns a reference to the @c SystemManager
    /// @return A reference to the @c SystemManager
    inline SystemManager& getSystemManager() {
		return systemManager;
	}
private:
    std::unordered_map<std::type_index, std::unique_ptr<IComponentManager>> componentManagers;
    SystemManager systemManager;
};