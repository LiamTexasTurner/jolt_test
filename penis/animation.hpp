#pragma once
#include <glad/glad.h>
#include <common.hpp>
#include "scene.hpp"
#include <vector>
#include <span>
#include <string>

void FK(std::span<const BoneInfo> bones, std::span<TRS> out_pose);

void DeformMeshGPU(const Skeleton* skeleton, std::span<TRS> pose, GLuint* skinning_compute_shader);

