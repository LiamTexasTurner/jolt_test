#include "Camera.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <iostream>
#include "input.hpp"


Camera::Camera()
{

}

Camera::Camera(glm::vec3 in_translation,
               glm::vec3 in_up,
               glm::vec3 in_right,
               float in_yaw,
               float in_pitch,
               CamType in_cam_type) :
                     translation(in_translation),
                     up(in_up),
                     right(in_right),
                     yaw(in_yaw),
                     pitch(in_pitch),
                     cam_type(in_cam_type)
{
      world_up = glm::vec3(0,1,0);
      UpdateCameraVectors();
}
glm::mat4 Camera::GetViewMatrix()
{
      return glm::lookAt(translation, translation + front, up);
}

glm::mat4 Camera::GetWorldMat()
{
      glm::mat4 world_mat = glm::mat4();
      world_mat[0] = glm::vec4(right, 0.0f);
      world_mat[1] = glm::vec4(up, 0.0f);
      world_mat[2] = glm::vec4(front, 0.0f);
      world_mat[3] = glm::vec4(translation, 1.0f);
      return world_mat;
}


void Camera::UpdateCameraVectors()
{

      front = glm::vec3(0.0f);
      front.x = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
      front.y = sin(glm::radians(pitch));
      front.z = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
      front = glm::normalize(front);

      right = glm::normalize(glm::cross(front, world_up));
      up = glm::normalize(glm::cross(right, front));
}

void Camera::UpdateFromInput(PlayerInput& player_input, float dt)
{
      MouseData& mouse_data = player_input.mouse_data;
      KeyInputs& key_inputs = player_input.key_inputs;

      //mouse
      yaw -= mouse_data.dx *  sensitivity;
      pitch += mouse_data.dy *  sensitivity;
      mouse_data.dx = 0.0f;
      mouse_data.dy = 0.0f;

      //key
      switch (cam_type)
      {
            case CamType::flying:
            {
                  if(key_inputs.W.is_down) translation += front * dt * flying_speed;
                  if(key_inputs.S.is_down) translation -= front * dt * flying_speed;
                  if(key_inputs.D.is_down) translation += right * dt * flying_speed;
                  if(key_inputs.A.is_down) translation -= right * dt * flying_speed;
                  if(key_inputs.E.is_down) translation += up * dt * flying_speed;
                  if(key_inputs.Q.is_down) translation -= up * dt * flying_speed;
                  
            } break;
      }
      
      UpdateCameraVectors();
      
      
}
