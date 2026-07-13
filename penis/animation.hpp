#pragma once
#include <glad/glad.h>
#include <common.hpp>
#include "scene.hpp"
#include <vector>
#include <span>
#include <string>

struct AnimationClip
{
      int index = -1;
      int frame_count = 0.0f;
      float current_time = 0.0f;
      float play_rate = 1.0f;
};

void FK(std::span<const BoneInfo> bones, std::span<TRS> out_pose);

void DeformMeshGPU(const Skeleton* skeleton, std::span<TRS> pose, GLuint* skinning_compute_shader);

