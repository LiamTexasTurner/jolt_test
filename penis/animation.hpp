#pragma once
#include <glad/glad.h>
#include <common.hpp>
#include "scene.hpp"
#include <vector>
#include <string>


void FK(std::span<const BoneInfo> bones, std::span<TRS> out_pose);

