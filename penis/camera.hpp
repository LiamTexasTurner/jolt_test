#pragma once

#include <glm/glm.hpp>

class Camera
{
public:

      Camera();
      
      Camera(glm::vec3 in_translation,
             glm::vec3 in_up,
             glm::vec3 in_right,
             float in_yaw,
             float in_pitch);


      
      glm::mat4 GetViewMatrix();
      glm::mat4 GetWorldMat();
      void updateCameraVectors();
      
      glm::vec3 translation = glm::vec3(0.0f);
      glm::vec3 front       = glm::vec3(0.0f);
      glm::vec3 up          = glm::vec3(0.0f);
      glm::vec3 right       = glm::vec3(0.0f);
      glm::vec3 world_up    = glm::vec3(0.0f);

      glm::vec3 offset      = glm::vec3(0.0f);

      float yaw   = 0.0f;
      float pitch = 0.0f;

      float sensitivity = 1.0f;

};
