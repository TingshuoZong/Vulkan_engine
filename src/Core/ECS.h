#pragma once

#include <cstdint>
#include <iostream>
#include <typeindex>
#include <unordered_map>
#include <vector>

using Entity = std::uint32_t;
constexpr Entity INVALID_ENTITY = 0;

struct IComponentManager {
    virtual ~IComponentManager() = default;
    virtual void remove_component(Entity entity) = 0;
};

template <typename T>
class ComponentManager: public IComponentManager {
public:
    void add_component(Entity entity, T& component) {
        if (entity_to_component_index.contains(entity)) {
            components[entity_to_component_index[entity]] = component;
	    } else {
            entity_to_component_index[entity] = components.size();
	        component_index_to_entity.push_back(entity);
	        components.push_back(component);
	    }
    }

    T* get_component(Entity entity) {
        if (entity_to_component_index.contains(entity)) {
            uint32_t component_index = entity_to_component_index[entity];
            return &components[component_index];
        } else return nullptr;
    }

    void remove_component(Entity entity) override {
        if (!entity_to_component_index.contains(entity)) {
            std::cerr << "Error: Trying to remove nonexistent component from entity\n";
            return;
        }

        auto remove_component = entity_to_component_index.find(entity);

        uint32_t remove_index = remove_component->second;
        uint32_t last_index = static_cast<uint32_t>(components.size()) - 1;
        Entity last_entity = component_index_to_entity[last_index];

        // Swap the last element and removed element to preserve dense packing
        components[remove_index] = std::move(components[last_index]);
        component_index_to_entity[remove_index] = last_entity;
        entity_to_component_index[last_entity] = remove_index;

        // Remove the last element now it's swapped with removed element
        components.pop_back();
        component_index_to_entity.pop_back();
        entity_to_component_index.erase(remove_component);
    }

    std::vector<T>& get_raw_component_list() {
        return components;
    }
private:
    std::vector<T> components;
    std::unordered_map<Entity, uint32_t> entity_to_component_index;
    std::vector<Entity> component_index_to_entity;
};


static Entity next_entity_id = 1;

class EntityManager {
public:
    static Entity create_entity() {
        next_entity_id++;
        return next_entity_id - 1;
    }

    template<typename T>
    ComponentManager<T>& get_component_manager() {
        const auto typeIndex = std::type_index(typeid(T));
        const auto componentManager = component_managers.find(typeIndex);

        if (componentManager == component_managers.end()) {
            auto manager = std::make_unique<ComponentManager<T>>();

            ComponentManager<T>* ptr = manager.get();
            component_managers[typeIndex] = std::move(manager);

            return *ptr;
        }
        return *static_cast<ComponentManager<T>*>(componentManager->second.get());
    }
private:
    std::unordered_map<std::type_index, std::unique_ptr<IComponentManager>> component_managers;
};