#pragma once
#include <stdint.h>
#include "arena.hpp"
#include "common.hpp"
#include "penis_mesh.hpp"
#include "packed_freelist.h"
#include <vector>
#include <unordered_map>
#include <string>



struct AnimationClip
{
      uint32_t ID = 0;
      int frame_count = 0.0f;
      float current_time = 0.0f;
      float play_rate = 1.0f;
};

struct StateMachine
{
      
};

struct AnimationGraph
{      
      

      AnimationGraph() = default;

      AnimationGraph(packed_freelist<Animation>* in_animations, int in_bone_count, uint32_t in_skeleton_ID, std::vector<uint32_t> in_anim_IDs) :
            animations(in_animations),
            bone_count(in_bone_count),
            skeleton_ID(in_skeleton_ID),
            anim_IDs(in_anim_IDs)
      {
            clips.resize(in_anim_IDs.size());
            int i = 0;
            for(uint32_t ID : in_anim_IDs)
            {
                  Animation& anim = (*animations)[ID];
                  std::cout << anim.name << std::endl;
                  anim_map.emplace(anim.name, ID);
                  clips[i].frame_count = anim.frame_count;
                  clips[i].ID = i;
                  i++;
            }
      }

      packed_freelist<Animation>* animations;
      std::vector<uint32_t> anim_IDs;
      std::vector<TRS> out_pose;
      std::vector<AnimationClip> clips;
      std::unordered_map<std::string, uint32_t> anim_map;

      uint32_t skeleton_ID;
      
      int bone_count;
};
void TickAnimGraph(Arena& arena, AnimationGraph& graph, float dt);
