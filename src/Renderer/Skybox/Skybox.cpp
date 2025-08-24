#include "Skybox.h"

Skybox::Skybox(daxa::Device* device, const std::shared_ptr<daxa::RasterPipeline> &pipeline, daxa::ImageViewId skybox_view, daxa::SamplerId sampler)
	: device(device),pipeline(pipeline), skybox_view(skybox_view), sampler(sampler) {

}