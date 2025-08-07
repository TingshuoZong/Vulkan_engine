#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include "Core/ECS/ECS.h"

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <variant>

struct TransformUpdatedMessage {
    Entity entity_id;
    glm::mat4 model_matrix;
};

using TransformMessage = std::variant<TransformUpdatedMessage>;