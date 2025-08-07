#pragma once

#include <memory>
#include <typeindex>
#include <unordered_map>

#include "ECS_Types.h"
#include "ECS_Messages.h"

class SystemManager {
public:
    template <typename T, typename... Args>
    inline void registerSystem (Args&... args) {
        auto system = std::make_unique<T> (args...);
        systems[std::type_index(typeid(T))] = std::move(system);
    }

    template <typename T>
    inline T& getSystem() {
        auto system = systems.find(std::type_index(typeid(T)));
        if (system != systems.end())
            return *dynamic_cast<T*>(system->second.get());
        else
            throw std::runtime_error("Error: System not found");
    }

    inline void updateSystems() {
        for (auto& system : systems) {
            system.second->update();
        }
    }
private:
    std::unordered_map<std::type_index, std::shared_ptr<ISystem>> systems;
};