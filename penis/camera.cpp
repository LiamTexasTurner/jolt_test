#include "Camera.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <iostream>


Camera::Camera()
{

}

Camera::Camera(glm::vec3 in_translation,
               glm::vec3 in_up,
               glm::vec3 in_right,
               float in_yaw,
               float in_pitch) :
                     translation(in_translation),
                     up(in_up),
                     right(in_right),
                     yaw(in_yaw),
                     pitch(in_pitch)
{
      world_up = glm::vec3(0,1,0);
      updateCameraVectors();
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


void Camera::updateCameraVectors()
{

      front = glm::vec3(0.0f);
      front.x = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
      front.y = sin(glm::radians(pitch));
      front.z = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
      front = glm::normalize(front);

      right = glm::normalize(glm::cross(front, world_up));
      up = glm::normalize(glm::cross(right, front));
}
