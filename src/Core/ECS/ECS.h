#pragma once

#include "ECS_Entity.h"
#include "ECS_Component.h"

/// @brief The ecs namespace has a EntityManager in the global scope as a singleton
/// @see ECS_DESIGN for ECS design suggestion/guidelines
namespace ecs {
    inline EntityManager entityManager;

    /// @brief Forwards getComponent to the appropriate @ref ComponentManager; this exists to reduce boilderplate
    /// @tparam T This is used to select the @ref ComponentManager
    /// @param entity EntityID of the entity to get the component for
    /// @return A pointer to the component
    template <typename T>
    inline T* getComponent(Entity entity) {
        return entityManager.getComponentManager<T>().getComponent(entity);
    }

    /// @brief Gets the appropriate @ref ComponentManager
    /// @tparam T This is used to select the @ref ComponentManager
    /// @param entity EntityID of the entity to get the @ref ComponentManager for; this exists to reduce boilderplate
    /// @return A reference to the @ref ComponentManager
    template <typename T>
    inline ComponentManager<T>& getComponentManager() {
        return entityManager.getComponentManager<T>();
    }

    /// @brief Forwards getSytem to the appropriate @ref SystemManager; this exists to reduce boilderplate
    /// @tparam T This is used to select the @ref SystemManager
    /// @param entity EntityID of the entity to get the system for
    /// @return A pointer to the system
    template <typename T>
    inline T* getSystem() {
        return entityManager.getSystemManager().getSystem();
    }

    /// @brief Gets the appropriate @ref SystemManager
    /// @tparam T This is used to select the @ref SystemManager
    /// @param entity EntityID of the entity to get the @ref SystemManager for; this exists to reduce boilderplate
    /// @return A reference to the @ref SystemManager
    inline SystemManager& getSystemManager() {
        return entityManager.getSystemManager();
    }

    /// @brief Forwards to @ref EntityManager::SystemManager; this exists to reduce boilderplate
    inline void updateSystems() {
        entityManager.getSystemManager().updateSystems();
    }
}