#pragma once

#include <memory>
#include <typeindex>
#include <unordered_map>

#include "ECS_Types.h"
#include "ECS_Messages.h"

/// @brief Manages the systems of all of the entities
class SystemManager {
public:
    /**
     * @brief Reigsters a @c SystemManager
     * 
     * This is different to a @ref ComponentManager because the there is only one @c SystemManager as there cannot be multiple instances of a system
     * 
     * @tparam T The type of the system you want to register
     * @tparam Args Arguments forwarded to the constructor of the system
     * @param args Arguments forwarded to the constructor of the system
     */
    template <typename T, typename... Args>
    inline void registerSystem (Args&... args) {
        auto system = std::make_unique<T> (args...);
        systems[std::type_index(typeid(T))] = std::move(system);
    }

    /// @brief Returns a reference to the system based off of the type of it
    /// @tparam T The type of the system to be returned
    /// @return A reference to the system
    template <typename T>
    inline T& getSystem() {
        auto system = systems.find(std::type_index(typeid(T)));
        if (system != systems.end())
            return *dynamic_cast<T*>(system->second.get());
        else
            throw std::runtime_error("Error: System not found");
    }

    /// @brief Calls @c update on all of the systems which is defined by @ref ISystem which all systems should inherit from
    inline void updateSystems() {
        for (auto& system : systems) {
            system.second->update();
        }
    }
private:
    std::unordered_map<std::type_index, std::shared_ptr<ISystem>> systems;
};