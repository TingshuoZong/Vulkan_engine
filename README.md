# Vulkan_engine

This is just a toy project I am working on to learn C++ and graphics programming.

**Note:** There is currently a bug that glm is marked as a subrepository but it should be vendored so to build you must supply your own glm inside /lib/glm

## Install/Build

You can use any generator you want (note for Visual Stuido you must have at least 2022 installed for daxa), in the example it is using Ninja

'''
git clone https://github.com/TingshuoZong/Vulkan_engine.git
mkdir .build && cd .build
cmake .. -G "Ninja"
ninja
'''

## ECS

The ECS is the main way game code will interact with the game engine, components such as `ManagedMesh` and `TransformComponent` allow for common functions such as rendering objects.

### Useage

The ECS exists as a singleton accessed through the `ecs` namespace.

### **IMPORTANT:** Please read ECS_DESIGN.md if extending ECS

## Low-level systems

Apart from the ECS it is also possible to interact with and extend the engine's systems by directly interacting with the engine's low level systems and abstractions.

### glTF Loading

Then game engine uses glTFs to store level data, these can be editied in separate editors i.e. Blender.

**Note:** Meshes in the engine are equivalent to primitives in glTFs
