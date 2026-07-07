#pragma once

#include <glm/glm.hpp>

class PlayerInput;

enum class CamType
{
      flying,
      third_person
};

class Camera
{
public:

      Camera();
      
      Camera(glm::vec3 in_translation,
             glm::vec3 in_up,
             glm::vec3 in_right,
             float in_yaw,
             float in_pitch,
             CamType in_cam_type);


      CamType cam_type;
      glm::mat4 GetViewMatrix();
      glm::mat4 GetWorldMat();

      void UpdateFromInput(PlayerInput& mouse_data, float dt);
      void UpdateCameraVectors();
      
      glm::vec3 translation = glm::vec3(0.0f);
      glm::vec3 front       = glm::vec3(0.0f);
      glm::vec3 up          = glm::vec3(0.0f);
      glm::vec3 right       = glm::vec3(0.0f);
      glm::vec3 world_up    = glm::vec3(0.0f);

      glm::vec3 offset      = glm::vec3(0.0f);

      float yaw   = 0.0f;
      float pitch = 0.0f;

      float sensitivity = 0.02f;

      float flying_speed = 4.0f;

};
