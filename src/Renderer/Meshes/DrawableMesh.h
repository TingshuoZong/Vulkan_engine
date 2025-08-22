#pragma once

#include "shared.inl"  // For UniformBufferObject or PushConstant

#include <daxa/daxa.hpp>
#include <daxa/utils/task_graph.hpp>

#include "Tools/Model_loader.h"

constexpr size_t MAX_INSTANCE_COUNT = 1024;

/**
 * @brief Holds the buffer of the actual vertex and index data and offsets for the aggragate buffers
 * 
 * There is a difference between how meshes work in the low-level parts of the engine vs. high-level (ecs) part
 * 
 * Low-level: Each mesh has separate mesh data (indicies and verticies) and instance data \n
 * High-level: @ref Manage dMesh abstracts the concept of mesh data vs. instance data away, replacing it with a "master" mesh and instanced mesh relationship more similar to other game engines
 * 
 * @ref DrawGroups are low-level abstractions to help with aggrgating buffers and indirect rendering \n
 * @ref MeshManager is what actually "owns" the mesh though it is stored as a @c std::shared_ptr this helps with managing the relationships with the @ref ManagedMesh abstraction but can be standalone \n
 * @ref ManagedMesh are ECS components that have the idea of meshes vs. instance data abstracted away, they can be either instanced or not instanced, if they are instanced they need both instance data and mesh data, if they are instanced they only hold instance data
 * 
 */
struct DrawableMesh {
    daxa::Device* device = nullptr;

    std::uint32_t vertex_offset;
    std::uint32_t index_offset;
    std::uint32_t instance_offset;

    std::vector<meshRenderer::Vertex> verticies;
    std::vector<uint32_t> indicies;

    std::vector<std::uint32_t> instance_data_offsets;

    size_t drawGroupIndex;

    std::uint32_t vertex_count;
    std::uint32_t index_count;

    std::vector<meshRenderer::PerInstanceData> instance_data;

    std::string name;

    /// @brief moves the vertex and index data of @c parsedPrimitive using @c std::move into the atual @c DrawableMesh
    /// @param parsedPrimitive The @ref ParsedPrimitive that comes from the model loader
    /// @param name The debug name that will be used to create the names for the tasks and buffers for daxa
    DrawableMesh(ParsedPrimitive parsedPrimitive, std::string name)
        :name(name) {
        vertex_count = parsedPrimitive.vertexCount;
        index_count = parsedPrimitive.indexCount;

        verticies = std::move(parsedPrimitive.vertices);
        indicies = std::move(parsedPrimitive.indices);
    }
};
