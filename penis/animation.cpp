#include "animation.hpp"
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include "preamble.glsl"

void FK(std::span<const BoneInfo> bones, std::span<TRS> out_pose)
{
      for(int i = 1; i < bones.size(); i++)
      {
            int bone_index = bones[i].index;
            int parent_index = bones[i].parent;
    
            glm::vec4 parent_position = out_pose[parent_index].translation;
            glm::quat parent_rotation = out_pose[parent_index].rotation;
            
            glm::vec4 local_position = out_pose[bone_index].translation;
            glm::vec4 ls_to_ms_position = (parent_rotation * local_position) + parent_position;

            glm::quat local_rotation = out_pose[bone_index].rotation;
            glm::quat ls_to_ms_rotation = parent_rotation * local_rotation;

            out_pose[bone_index].translation = ls_to_ms_position;

            out_pose[bone_index].rotation = ls_to_ms_rotation;
      }
}

void DeformMeshGPU(const Skeleton* skeleton, std::span<TRS> pose, GLuint* skinning_compute_shader)
{
      glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SKINNING_COMPUTE_INV_BIND_POSE_BINDING, skeleton->inv_bind_pose_SSBO);
      glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SCENE_BONE_MAT_SSBO_BINDING, skeleton->bone_transform_SSBO);
      
      GLsizeiptr buffer_size = sizeof(TRS) * skeleton->bone_count;
      glBindBuffer(GL_SHADER_STORAGE_BUFFER, skeleton->anim_trs_SSBO);
      glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, buffer_size, pose.data());
      glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SKINNING_COMPUTE_TRS_BINDING, skeleton->anim_trs_SSBO);      
      
      glUseProgram(*skinning_compute_shader);
      glDispatchCompute((skeleton->bone_count + SKINNING_GROUP_SIZE_X - 1) / SKINNING_GROUP_SIZE_X, 1, 1);
      glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}
