#pragma once

#include <cstdint>
#include <iostream>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include "ECS_Types.h"

template <typename T>
class ComponentManager: public IComponentManager {
public:
    void addComponent(Entity entity, T&& component) {
        if (entityToComponentIndex.contains(entity)) {
            components[entityToComponentIndex[entity]] = component;
        } else {
            entityToComponentIndex[entity] = components.size();
            componentIndexToEntity.push_back(entity);
            components.push_back(component);
        }
    }

    T* getComponent(Entity entity) {
        if (entityToComponentIndex.contains(entity)) {
            uint32_t component_index = entityToComponentIndex[entity];
            return &components[component_index];
        } else return nullptr;
    }

    void removeComponent(Entity entity) override {
        if (!entityToComponentIndex.contains(entity)) {
            std::cerr << "Error: Trying to remove nonexistent component from entity\n";
            return;
        }

        auto remove_component = entityToComponentIndex.find(entity);

        uint32_t remove_index = remove_component->second;
        uint32_t last_index = static_cast<uint32_t>(components.size()) - 1;
        Entity last_entity = componentIndexToEntity[last_index];

        // Swap the last element and removed element to preserve dense packing
        components[remove_index] = std::move(components[last_index]);
        componentIndexToEntity[remove_index] = last_entity;
        entityToComponentIndex[last_entity] = remove_index;

        // Remove the last element now it's swapped with removed element
        components.pop_back();
        componentIndexToEntity.pop_back();
        entityToComponentIndex.erase(remove_component);
    }

    std::vector<T>& get_raw_component_list() {
        return components;
    }
private:
    std::vector<T> components;
    std::unordered_map<Entity, uint32_t> entityToComponentIndex;
    std::vector<Entity> componentIndexToEntity;
};
