#pragma once

#include "skybox_rendering_shared.inl"

#include <memory>

#include <daxa/pipeline.hpp>
#include <daxa/utils/task_graph.hpp>

class Skybox {
public:
	daxa::Device* device;
	std::shared_ptr<daxa::RasterPipeline> pipeline;

	daxa::ImageViewId skybox_view;
	daxa::SamplerId sampler;

	daxa::BufferId vertex_buffer_id;
	daxa::BufferId index_buffer_id;

	daxa::TaskBuffer task_vertex_buffer;
	daxa::TaskBuffer task_index_buffer;

	Skybox() = default;
	explicit Skybox(daxa::Device* device, const std::shared_ptr<daxa::RasterPipeline> &pipeline, daxa::ImageViewId skybox_view, daxa::SamplerId sampler);

	/*void uploadBuffers(daxa::TaskGraph& tg) {
		vertex_buffer_id = device->create_buffer({
		.size = 24 * sizeof(skyboxRenderer::Vertex),
		.name = "skybox vertex buffer"
			});

		task_vertex_buffer = daxa::TaskBuffer({
			.initial_buffers = {.buffers = std::span{&vertex_buffer_id, 1}},
			.name = "skybox task vertex buffer"
			});

		index_buffer_id = device->create_buffer({
			.size = 36 * sizeof(uint32_t),
			.name = "skybox index buffer"
			});

		task_index_buffer = daxa::TaskBuffer({
			.initial_buffers = {.buffers = std::span{&index_buffer_id, 1}},
			.name = "skybox task index buffer"
			});

		tg.use_persistent_buffer(task_vertex_buffer);
		tg.use_persistent_buffer(task_index_buffer);

		tg.add_task({
			.attachments = {
				daxa::inl_attachment(daxa::TaskBufferAccess::TRANSFER_WRITE, this->task_vertex_buffer),
				daxa::inl_attachment(daxa::TaskBufferAccess::TRANSFER_WRITE, this->task_index_buffer),
			},
			.task = [=, this](daxa::TaskInterface ti) {
				auto vertex_staging = ti.device.create_buffer({
						   .size = SKYBOX_CUBE_VERTICIES.size() * sizeof(skyboxRenderer::Vertex),
						   .allocate_info = daxa::MemoryFlagBits::HOST_ACCESS_RANDOM,
						   .name = "skybox vertex staging buffer",
				});
				ti.recorder.destroy_buffer_deferred(vertex_staging);
				auto* vertex_ptr = ti.device.buffer_host_address_as<skyboxRenderer::Vertex>(vertex_staging).value();
				memcpy(vertex_ptr, SKYBOX_CUBE_VERTICIES.data(), SKYBOX_CUBE_VERTICIES.size() * sizeof(skyboxRenderer::Vertex));

				ti.recorder.copy_buffer_to_buffer({
					.src_buffer = vertex_staging,
					.dst_buffer = ti.get(this->task_vertex_buffer).ids[0],
					.size = SKYBOX_CUBE_VERTICIES.size() * sizeof(skyboxRenderer::Vertex),
				});

				auto index_staging = ti.device.create_buffer({
				.size = SKYBOX_CUBE_INDICIES.size() * sizeof(uint32_t),
				.allocate_info = daxa::MemoryFlagBits::HOST_ACCESS_RANDOM,
				.name = "skybox index staging buffer",
					});
				ti.recorder.destroy_buffer_deferred(index_staging);
				auto* index_ptr = ti.device.buffer_host_address_as<uint32_t>(index_staging).value();
				std::memcpy(index_ptr, SKYBOX_CUBE_INDICIES.data(), SKYBOX_CUBE_INDICIES.size() * sizeof(uint32_t));

				ti.recorder.copy_buffer_to_buffer({
					.src_buffer = index_staging,
					.dst_buffer = ti.get(this->task_index_buffer).ids[0],
					.size = SKYBOX_CUBE_INDICIES.size() * sizeof(uint32_t),
				});
			}
		});
	}*/
private:
};
