#pragma once

#include <vector>

#include <daxa/device.hpp>
#include <daxa/gpu_resources.hpp>

#include "TextureHandle.h"

/**
 * @brief Stores and manages all the GPU side textures
 * 
 * All the textures stored here are @c Daxa::ImageViewId or @ref TextureHandle so there is no need to purge them they are just references
 * This class is where you will get the actual @c daxa::ImageViewId instead of from the individal @ref TestureHandle themselves
 * This class will typically just be used as a singleton though this is not strictly necessary i.e. if you want to manage multiple textures such as albedo, normals and metalness
 * All textures are bindless
 * 
 */
class TextureManager {
public:
    std::vector<TextureHandle> textures;
    std::vector<daxa::ImageViewId> views;

    /// @brief Creates an instance of TextureManager
    /// @param device A reference to the @c daxa::Device to upload the textures to
    /// @param loop_task_graph A reference to a @c daxa::TaskGraph that will be used to upload the textures
    TextureManager(daxa::Device& device, daxa::TaskGraph& loop_task_graph, BulkTextureUploadManager& bulkTextureUploadManager)
        : device(device), loop_task_graph(loop_task_graph), bulkTextureUploadManager(bulkTextureUploadManager) {}

    /// @brief Sreams in textures from storage devices
    /// @param fileName The filename, the actual path to find the textures is found in @ref TextureHandle.h
    /// @param debug_name The debug name that will be used for the buffers and tasks to upload it
    inline void stream_texture_from_memory(std::string fileName, std::string debug_name) {
        textures.push_back(TextureHandle(device));
        textures[textures.size() - 1].stream_texture_from_memory(fileName, debug_name, bulkTextureUploadManager);
        views.push_back(textures[textures.size() - 1].load_texture(loop_task_graph));
    }
    /// @brief Streams in textures from data loaded in ram in the form of @c tinygltf::Image
    /// @param gltf_image A reference to the @c tinygltf::Image to upload
    /// @param debug_name The debug name that will be used for the buffers and tasks to upload it
    inline void stream_texture_from_data(const tinygltf::Image& gltf_image, std::string debug_name) {
        textures.push_back(TextureHandle(device));
        textures[textures.size() - 1].stream_texture_from_data(gltf_image, debug_name, bulkTextureUploadManager);
        views.push_back(textures[textures.size() - 1].load_texture(loop_task_graph));
    }
private:
    daxa::Device& device;
    daxa::TaskGraph& loop_task_graph;
    BulkTextureUploadManager& bulkTextureUploadManager;
};
