#pragma once

#include "shared.inl"  // For UniformBufferObject or PushConstant

#include <daxa/daxa.hpp>
#include <daxa/utils/task_graph.hpp>

#include "Tools/Model_loader.h"


constexpr size_t MAX_INSTANCE_COUNT = 1024;

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

    DrawableMesh(ParsedPrimitive parsedPrimitive, std::string name)
        :name(name) {
        vertex_count = parsedPrimitive.vertexCount;
        index_count = parsedPrimitive.indexCount;

        verticies = std::move(parsedPrimitive.vertices);
        indicies = std::move(parsedPrimitive.indices);
    }
};
