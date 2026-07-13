#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

struct TRS
{
      glm::vec4 translation;
      glm::quat rotation;
      glm::vec4 scale;
};

