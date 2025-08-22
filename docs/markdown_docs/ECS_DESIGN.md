# ECS Design

## Entities
The type `Entity` is just an alias for `uint32_t`

## Components
All components inherit from `IComponent` which defines `thisEntity` and `initialize(Entity initalizedEntity)`, the `IComponent` interface class

**Important:** The use of `this` is **unsafe** to use within components and systems outside the `initialize` function and should ideally, never be used.
This is because components and systems are stored in heap-allocated arrays and may be reallocated at any time.

If a pointer to the current component, you should use the `getComponent` method of the appropriate component manager and pass in the entityID of the entity that owns the component

### Component Design
Components may contain methods, however, these methods may not mutate other components.
To mutate other components either messages (targeted at one system) or broadcasts (messages handled by the systemManager) can be sent.

# Systems

# Messages and Broadcasts