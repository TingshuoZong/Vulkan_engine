#pragma once

using Entity = std::uint32_t;
constexpr Entity INVALID_ENTITY = 0;

struct IComponentManager {
    virtual ~IComponentManager() = default;
    virtual void removeComponent(Entity entity) = 0;
};

struct IComponent {
    Entity thisEntity;

    virtual ~IComponent() = default;
    virtual void initialize(Entity initializedEntity) { thisEntity = initializedEntity; };
};


struct ISystem {
    virtual ~ISystem() = default;
    virtual void update() = 0;
};