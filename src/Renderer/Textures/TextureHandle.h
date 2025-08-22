#pragma once

#include <daxa/daxa.hpp>
#include <daxa/utils/pipeline_manager.hpp>
#include <daxa/utils/task_graph.hpp>

#include <tiny_gltf.h>
#include <stb_image.h>

#include <string>
constexpr const char* TEXTURE_PATH = "C:/dev/Engine_project/assets/textures/";

struct UploadData {
    daxa::TaskBuffer task_texture_staging;
    daxa::TaskImage task_texture_image;
    daxa::ImageId image;

    uint32_t width;
    uint32_t height;
};

/**
 * @brief The manager that acually queues up all the textures to be uploaded and submits its to the GPU
 * 
 * @c BulkTextureUploadManager will typically be used as a singleton and is used to batch texture uploads to decrease the number of tasks
 * @ref TextureHandle loading
 * 
 */
class BulkTextureUploadManager {
public:
    void submitUpload(const UploadData& uploadData) { uploads.push_back(uploadData); }
    std::vector <daxa::ImageViewId> bulkUploadTextures(daxa::TaskGraph& taskGraph, const std::string& name);
private:
    std::vector<UploadData> uploads;
    std::vector<daxa::ImageViewId> views;
};

struct TextureHandle {
    int width;
    int height;
    int channels;

    std::string name;
    daxa::Device& device;

    daxa::ImageViewId texture_view;

    daxa::ImageId image;
    daxa::TaskImage task_texture_image;
    daxa::BufferId texture_staging_buffer;
    daxa::TaskBuffer task_texture_staging;

    // TODO: mip maps?
    // TODO: create a state handler to handle: steraming from disc -> compressed in cpu ram -> decompressed and uploaded into vram
    // TODO: create an abstraction to remove from vram and uncache from cpu ram

    explicit TextureHandle(daxa::Device& device);
	void cleanup() const;

    void stream_texture_from_memory(const std::string& fileName, const std::string &debug_name, BulkTextureUploadManager& manager);
    void stream_texture_from_data(const tinygltf::Image& gltf_image, std::string debug_name, BulkTextureUploadManager& manager);
    inline void load_textures_into_buffers(const stbi_uc* pixels, uint32_t size_bytes, BulkTextureUploadManager& manager);
    daxa::ImageViewId load_texture(daxa::TaskGraph& task_graph);
};