#pragma once

#include <daxa/daxa.hpp>
#include <daxa/utils/pipeline_manager.hpp>
#include <daxa/utils/task_graph.hpp>

#include <string>
constexpr const char* TEXTURE_PATH = "C:/dev/Engine_project/assets/textures/";

struct TextureHandle {
    int width;
    int height;
    int channels;

    std::string fileName;   // TODO: rename to filename
    daxa::Device& device;

    daxa::ImageViewId texture_view;

    daxa::ImageId image;
    daxa::TaskImage task_texture_image;
    daxa::BufferId texture_staging_buffer;
    daxa::TaskBuffer task_texture_staging;

    // TODO: mip maps?
    // TODO: create a state handler to handle: steraming from disc -> compressed in cpu ram -> decompressed and uploaded into vram
    // TODO: create an abstraction to remove from vram and uncache from cpu ram

    TextureHandle(daxa::Device& device, std::string fileName);
	void cleanup();

    void stream_texture_from_memory();
    daxa::ImageViewId load_texture(daxa::TaskGraph& loop_task_graph);
};