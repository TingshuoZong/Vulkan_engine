# Vulkan_engine

This is just a toy project I am working on to learn C++ and graphics programming.

## ECS

The ECS is the main way game code will interact with the game engine, components such as `ManagedMesh` and `TransformComponent` allow for common functions such as rendering objects.

### Useage

The ECS exists as a singleton accessed through the `ecs` namespace.

### **IMPORTANT:** Please read ECS_DESIGN.md if extending ECS

## Low-level systems

Apart from the ECS it is also possible to interact with and extend the engine's systems by directly interacting with the engine's low level systems and abstractions.