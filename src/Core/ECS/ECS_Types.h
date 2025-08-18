#pragma once
#include <cstdint>

/// @brief @c Entity is just an alias of @c std::uint32_t which is just the id of the entity
/// @see ECS_DESIGN for ECS design suggestion/guidelines
using Entity = std::uint32_t;
constexpr Entity INVALID_ENTITY = 0;

struct IComponentManager {
    virtual ~IComponentManager() = default;
    virtual void removeComponent(Entity entity) = 0;
};

/**
 * @brief Every component should inherit from this interface
 * @warning variable @c thisEntity is the entityID of the entity that owns the component, it should always be used in conjunction with @ref ComponentManager::getComponent to get a reference of itself and the @c this pointer should never be used
 */
struct IComponent {
    Entity thisEntity;

    virtual ~IComponent() = default;
    virtual void initialize(Entity initializedEntity) { thisEntity = initializedEntity; };
};


struct ISystem {
    virtual ~ISystem() = default;
    virtual void update() = 0;
};