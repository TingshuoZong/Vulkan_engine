#pragma once

#include <iostream>
#include <unordered_map>
#include <vector>

#include "ECS_Types.h"

/**
 * @brief Holds a @c std::vector of all the components of its type
 * 
 * Every component has a component manager of that type, it is what you use to access and modify the compoents themselves
 * @warning The @c this pointer should never be used inside of a component or system as they are stored directly in @c std::vector and may reallocate at anytime invalidating them
 * Caution should also be taken if you do anything within a component that may add another component like in @ref ManagedMesh::instanciate (ideally this should be done through a system)
 * @note Each entity may only have one of each type of component, trying to add a component when an entity already has a component of that type will result in the previous component being replaced by the new one
 * 
 * @tparam T The actual component that the manager will be managing
 * @see ECS_DESIGN for ECS design suggestion/guidelines
 */
template <typename T>
class ComponentManager: public IComponentManager {
public:
    /// @brief Adds component if the enttiy doesn't already have one of that type, if the entity does the previous component will be replaced
    /// @param entity The entity the component belongs to
    /// @param component A rvalue reference to the component, you should either construct the component in place or use @c std::move
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

    /// @brief Returns a pointer to the component of the given type that belongs to the entity
    /// @param entity Entities are just a @c uint32_t that represents that entity's id
    /// @return Returns a pointer to the component, a @c nullptr will be returned if the entity does not have a component of that type
    T* getComponent(Entity entity) {
        if (entityToComponentIndex.contains(entity)) {
            uint32_t component_index = entityToComponentIndex[entity];
            return &components[component_index];
        } else return nullptr;
    }

    /**
     * @brief Removes component of the given type from the entity, it will throw a @c std::cerr if the entity does not have a component of the given type
     * 
     * The way it removes a component is it swaps it to the last in the @c std::vector that stores all of the components, then it deletes the last element
     */
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

    /// @brief Returns a pointer of the @c std::vector that holds the components 
    std::vector<T>& get_raw_component_list() {
        return components;
    }

private:
    std::vector<T> components;
    std::unordered_map<Entity, uint32_t> entityToComponentIndex;
    std::vector<Entity> componentIndexToEntity;
};
