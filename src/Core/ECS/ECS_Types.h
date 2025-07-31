#pragma once

using Entity = std::uint32_t;
constexpr Entity INVALID_ENTITY = 0;

struct IComponentManager {
    virtual ~IComponentManager() = default;
    virtual void removeComponent(Entity entity) = 0;
};