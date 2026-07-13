#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

struct transform_penis
{
      glm::vec3 translation;
      glm::quat rotation;
      glm::vec3 scale;
};
struct trs
{
      glm::vec4 t;
      glm::vec4 r;
      glm::vec4 s;
};
