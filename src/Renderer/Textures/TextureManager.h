#pragma once

#include <vector>

#include <daxa/device.hpp>
#include <daxa/gpu_resources.hpp>

#include "TextureHandle.h"

class TextureManager {
public:
    std::vector<TextureHandle> textures;
    std::vector<daxa::ImageViewId> views;

    TextureManager(daxa::Device& device, daxa::TaskGraph& loop_task_graph)
        : device(device), loop_task_graph(loop_task_graph) {}

    inline void stream_texture_from_memory(std::string fileName, std::string debug_name) {
        textures.push_back(TextureHandle(device));
        textures[textures.size() - 1].stream_texture_from_memory(fileName, debug_name);
        views.push_back(textures[textures.size() - 1].load_texture(loop_task_graph));
    }
    inline void stream_texture_from_data(const tinygltf::Image& gltf_image, std::string debug_name) {
        textures.push_back(TextureHandle(device));
        textures[textures.size() - 1].stream_texture_from_data(gltf_image, debug_name);
        views.push_back(textures[textures.size() - 1].load_texture(loop_task_graph));
    }
private:
    daxa::Device& device;
    daxa::TaskGraph& loop_task_graph;
};
