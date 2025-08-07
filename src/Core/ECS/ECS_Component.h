#pragma once

#include <iostream>
#include <unordered_map>
#include <vector>

#include "ECS_Types.h"

template <typename T>
class ComponentManager: public IComponentManager {
public:
    void addComponent(Entity entity, T&& component) {
        if (entityToComponentIndex.contains(entity)) {
            components[entityToComponentIndex[entity]] = std::move(component);
            components[entityToComponentIndex[entity]].initialize(entity);
        } else {
            entityToComponentIndex[entity] = components.size();
            componentIndexToEntity.push_back(entity);
            components.push_back(std::move(component));
            // Add pointer to reverse lookup map
            components.back().initialize(entity);
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

        uint32_t removeIndex = remove_component->second;
        uint32_t lastIndex = static_cast<uint32_t>(components.size()) - 1;
        Entity lastEntity = componentIndexToEntity[lastIndex];

        // Get component pointers before swap so we can remap componentPtrToEntity
        T* removePtr = &components[removeIndex];
        T* lastPtr = &components[lastIndex];

        // Swap the last element and removed element to preserve dense packing
        components[removeIndex] = std::move(components[lastIndex]);
        componentIndexToEntity[removeIndex] = lastEntity;
        entityToComponentIndex[lastEntity] = removeIndex;

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
