#include "game_mode.hpp"
#include "scene.hpp"
#include "penis_jolt.hpp"
#include "renderer.hpp"
#include "entity.hpp"
#include "GLFW/glfw3.h"
#include "input.hpp"
#include "job_system.h"
#include <cereal/cereal.hpp>
#include <fstream>
#include <Jolt/Physics/Ragdoll/Ragdoll.h>

#include <iostream>

using namespace std;

class GameMode : public IGameMode
{
public:

      std::vector<Entity> entities;      
      Scene* m_scene;
      pJolt* m_jolt;
      IRenderer* m_renderer;
      GLFWwindow* m_window;


      uint32_t current_skybox;

      bool m_first_update;

      unsigned int m_last_update_tick;

      void Init(Scene* scene, pJolt* p_jolt, GLFWwindow* window, IRenderer* renderer) override
      {
            m_scene = scene;
      
            m_jolt = p_jolt;

            m_renderer = renderer;

            m_window = window;

            m_first_update = true;

            uint32_t chips_skeleton_ID = 0;
            vector<uint32_t> chips_animations;
            

            //skeletons
            {
                  SkeletonData skeleton_data;
                  std::ifstream is("../resources/skeletons/chips_rig.pskeleton", ios::binary);
                  cereal::BinaryInputArchive i_archive(is);
                  i_archive(skeleton_data);

                  chips_skeleton_ID = LoadSkeleton(*m_scene, skeleton_data);

                  chips_animations = LoadAnimations(*m_scene, "../resources/chips_2/animations/");
            }


            //Chips
            {
                  MeshData chips_mesh;
                  std::ifstream is("../resources/chips_2/bin/chips_2.pbin", ios::binary);
                  cereal::BinaryInputArchive i_archive(is);
                  i_archive(chips_mesh);

                  pSkeleton chips_skeleton = scene->skeletons[chips_skeleton_ID];
                  AnimationGraph chips_graph(&m_scene->animations,
                                             chips_skeleton.bone_count,
                                             chips_skeleton_ID,
                                             chips_animations,
                                             chips_skeleton.bone_info);                

                  uint32_t chips_ID = LoadSkeletalMesh(*m_scene, chips_mesh);



                  
                  
                  //Instances
                  for(int i = 0; i < 1; i++)
                  {
                        uint32_t new_instance_ID = 0;

                        if(!chips_mesh.skinned)
                        {
                              AddMeshInstance(*scene, chips_ID, &new_instance_ID);
                        }
                        else
                        {
                              AddSkinnedMeshInstance(*scene, chips_ID, &new_instance_ID, chips_graph);
                        }
                              
                        uint32_t new_transform_ID = scene->instances[new_instance_ID].transform_ID;
                        scene->transforms[new_instance_ID].translation = glm::vec3(i + 0,0,0);
                        // scene->transforms[new_instance_ID].rotation = glm::angleAxis(glm::radians(180.0f), glm::vec3(0,1,0));
                        entities.emplace_back(Entity(new_instance_ID));
                  }
            };

            //pilot
            // {
            //       MeshData mesh;
            //       std::ifstream is("../resources/pilot/bin/pilot.pbin", ios::binary);
            //       cereal::BinaryInputArchive i_archive(is);
            //       i_archive(mesh);
            //       uint32_t mesh_ID = LoadMesh(*m_scene, mesh);
            //       {
            //             uint32_t new_instance_ID = 0;

            //             if(!mesh.skinned)
            //             {
            //                   AddMeshInstance(*scene, mesh_ID, &new_instance_ID);
            //             }
            //             else
            //             {
            //                   AddSkinnedMeshInstance(*scene, mesh_ID, &new_instance_ID, std::move(chips_graph));
            //             }
                              
            //             uint32_t new_transform_ID = scene->instances[new_instance_ID].transform_ID;
            //             scene->transforms[new_instance_ID].translation = glm::vec3(0,1,0);
            //             scene->transforms[new_instance_ID].rotation = glm::angleAxis(glm::radians(180.0f), glm::vec3(0,1,0));
            //             entities.emplace_back(Entity(new_instance_ID));
            //       }                 
            // }
            //;

            
                  
            AddSkybox(*m_scene, &current_skybox);
            
            Camera main_camera(glm::vec3(0.0f, 2.0f, -10.0f), glm::vec3(0,1,0), glm::vec3(1,0,0), 0.0f, 0.0f, CamType::flying);
            m_scene->main_camera_ID = m_scene->cameras.insert((main_camera));

            m_renderer->UpdateBuffers(scene);
      }

      void HandleEvent() override
      {
            
      }

      void Update(float dt) override
      {
            GameData* gd = static_cast<GameData*>(glfwGetWindowUserPointer(m_window));
            PlayerInput& player_input = gd->player_input;
            KeyInputs& key_inputs = player_input.key_inputs;
            MouseData& mouse_data = player_input.mouse_data;

            if(!gd->cursor_enabled)
            {
                  Camera& main_camera = m_scene->cameras[m_scene->main_camera_ID];
                  main_camera.UpdateFromInput(player_input, dt);
                  
                  if(key_inputs.space.pressed)
                  {
                        RemoveInstance(*m_scene, entities.back().instance_ID);      
                  }
            }
            
            if(key_inputs.tab.pressed)
            {
                  m_renderer->DrawDebugQuad();
            }
            if(key_inputs.shift.pressed)
            {
                  m_renderer->toggle_skinned();
            }

            
            // for(int i = 0; i < m_scene->animation_graphs.size(); i++)
            // {
            //       AnimationGraph& graph = m_scene->animation_graphs[i];
            //       TickAnimGraphSerial(graph, dt);
                  
            // }
            
            pJobSystem::Dispatch(m_scene->animation_graphs.size(),
                                1,
                                [this, dt]
                                (JobDispatchArgs args, Arena& arena)
            {
                  AnimationGraph& graph = m_scene->animation_graphs[args.jobIndex];
                  TickAnimGraph(arena, graph, dt, args.jobIndex);
            });

            pJobSystem::Wait();
      

            
      }

      void PrePhysicsUpdate(float dt) override
      {
            pSkeleton& skeleton = m_scene->skeletons[0];
            AnimationGraph& graph = m_scene->animation_graphs[0];
            vector<TRS>& pose = graph.out_pose;
            Ragdoll* ragdoll = m_scene->ragdolls[0];

            int pelvis = skeleton.bone_name_index_map.find("pelvis")->second;
            int spine_03 = skeleton.bone_name_index_map.find("spine_03")->second;
            int spine_05 = skeleton.bone_name_index_map.find("spine_05")->second;
            int head = skeleton.bone_name_index_map.find("head")->second;
            
            
            
            m_jolt->mPhysicsSystem->GetBodyInterface().SetPositionAndRotation(ragdoll->GetBodyIDs()[0],
                                                                              glm_to_j_vec(pose[pelvis].translation),
                                                                              glm_to_j_quat(pose[pelvis].rotation),
                                                                              EActivation::DontActivate);

            m_jolt->mPhysicsSystem->GetBodyInterface().SetPositionAndRotation(ragdoll->GetBodyIDs()[1],
                                                                              glm_to_j_vec(pose[spine_03].translation),
                                                                              glm_to_j_quat(pose[spine_03].rotation),
                                                                              EActivation::DontActivate);

            m_jolt->mPhysicsSystem->GetBodyInterface().SetPositionAndRotation(ragdoll->GetBodyIDs()[2],
                                                                              glm_to_j_vec(pose[spine_05].translation),
                                                                              glm_to_j_quat(pose[spine_05].rotation),
                                                                              EActivation::DontActivate);
      
            m_jolt->mPhysicsSystem->GetBodyInterface().SetPositionAndRotation(ragdoll->GetBodyIDs()[3],
                                                                              glm_to_j_vec(pose[head].translation),
                                                                              glm_to_j_quat(pose[head].rotation),
                                                                              EActivation::DontActivate);
      
            int upperarm_l = skeleton.bone_name_index_map.find("upperarm_l")->second;
            int lowerarm_l = skeleton.bone_name_index_map.find("lowerarm_l")->second;

            glm::vec3 upperarm_l_pos = pose[upperarm_l].translation + ((pose[lowerarm_l].translation - pose[upperarm_l].translation) * 0.5f);
            glm::quat upperarm_l_rot = pose[upperarm_l].rotation * glm::angleAxis(glm::radians(90.0f), glm::vec3(0,0,1));


            m_jolt->mPhysicsSystem->GetBodyInterface().SetPositionAndRotation(ragdoll->GetBodyIDs()[4],
                                                                              glm_to_j_vec(upperarm_l_pos),
                                                                              glm_to_j_quat(upperarm_l_rot),
                                                                              EActivation::DontActivate);

            int upperarm_r = skeleton.bone_name_index_map.find("upperarm_r")->second;
            int lowerarm_r = skeleton.bone_name_index_map.find("lowerarm_r")->second;

            glm::vec3 upperarm_r_pos = pose[upperarm_r].translation + ((pose[lowerarm_r].translation - pose[upperarm_r].translation) * 0.5f);
            glm::quat upperarm_r_rot = pose[upperarm_r].rotation * glm::angleAxis(glm::radians(90.0f), glm::vec3(0,0,-1));


            m_jolt->mPhysicsSystem->GetBodyInterface().SetPositionAndRotation(ragdoll->GetBodyIDs()[5],
                                                                              glm_to_j_vec(upperarm_r_pos),
                                                                              glm_to_j_quat(upperarm_r_rot),
                                                                              EActivation::DontActivate);


            
            int hand_l = skeleton.bone_name_index_map.find("hand_l")->second;

            glm::vec3 lowerarm_l_pos = pose[lowerarm_l].translation + ((pose[hand_l].translation - pose[lowerarm_l].translation) * 0.5f);
            glm::quat lowerarm_l_rot = pose[lowerarm_l].rotation * glm::angleAxis(glm::radians(90.0f), glm::vec3(0,0,1));


            m_jolt->mPhysicsSystem->GetBodyInterface().SetPositionAndRotation(ragdoll->GetBodyIDs()[6],
                                                                              glm_to_j_vec(lowerarm_l_pos),
                                                                              glm_to_j_quat(lowerarm_l_rot),
                                                                              EActivation::DontActivate);


            int hand_r = skeleton.bone_name_index_map.find("hand_r")->second;

            glm::vec3 lowerarm_r_pos = pose[lowerarm_r].translation + ((pose[hand_r].translation - pose[lowerarm_r].translation) * 0.5f);
            glm::quat lowerarm_r_rot = pose[lowerarm_r].rotation * glm::angleAxis(glm::radians(90.0f), glm::vec3(0,0,1));


            m_jolt->mPhysicsSystem->GetBodyInterface().SetPositionAndRotation(ragdoll->GetBodyIDs()[7],
                                                                              glm_to_j_vec(lowerarm_r_pos),
                                                                              glm_to_j_quat(lowerarm_r_rot),
                                                                              EActivation::DontActivate);




            int thigh_l = skeleton.bone_name_index_map.find("thigh_l")->second;
            int calf_l = skeleton.bone_name_index_map.find("calf_l")->second;

            glm::vec3 thigh_l_pos = pose[thigh_l].translation + ((pose[calf_l].translation - pose[thigh_l].translation) * 0.5f);
            glm::quat thigh_l_rot = pose[thigh_l].rotation * glm::angleAxis(glm::radians(90.0f), glm::vec3(0,0,1));

            m_jolt->mPhysicsSystem->GetBodyInterface().SetPositionAndRotation(ragdoll->GetBodyIDs()[8],
                                                                              glm_to_j_vec(thigh_l_pos),
                                                                              glm_to_j_quat(thigh_l_rot),
                                                                              EActivation::DontActivate);

            

            int thigh_r = skeleton.bone_name_index_map.find("thigh_r")->second;
            int calf_r = skeleton.bone_name_index_map.find("calf_r")->second;

            glm::vec3 thigh_r_pos = pose[thigh_r].translation + ((pose[calf_r].translation - pose[thigh_r].translation) * 0.5f);
            glm::quat thigh_r_rot = pose[thigh_r].rotation * glm::angleAxis(glm::radians(90.0f), glm::vec3(0,0,1));

            m_jolt->mPhysicsSystem->GetBodyInterface().SetPositionAndRotation(ragdoll->GetBodyIDs()[9],
                                                                              glm_to_j_vec(thigh_r_pos),
                                                                              glm_to_j_quat(thigh_r_rot),
                                                                              EActivation::DontActivate);


            int foot_l = skeleton.bone_name_index_map.find("foot_l")->second;

            glm::vec3 calf_l_pos = pose[calf_l].translation + ((pose[foot_l].translation - pose[calf_l].translation) * 0.5f);
            glm::quat calf_l_rot = pose[calf_l].rotation * glm::angleAxis(glm::radians(90.0f), glm::vec3(0,0,1));

            m_jolt->mPhysicsSystem->GetBodyInterface().SetPositionAndRotation(ragdoll->GetBodyIDs()[10],
                                                                              glm_to_j_vec(calf_l_pos),
                                                                              glm_to_j_quat(calf_l_rot),
                                                                              EActivation::DontActivate);

            int foot_r = skeleton.bone_name_index_map.find("foot_r")->second;

            glm::vec3 calf_r_pos = pose[calf_r].translation + ((pose[foot_r].translation - pose[calf_r].translation) * 0.5f);
            glm::quat calf_r_rot = pose[calf_r].rotation * glm::angleAxis(glm::radians(90.0f), glm::vec3(0,0,1));

            m_jolt->mPhysicsSystem->GetBodyInterface().SetPositionAndRotation(ragdoll->GetBodyIDs()[11],
                                                                              glm_to_j_vec(calf_r_pos),
                                                                              glm_to_j_quat(calf_r_rot),
                                                                              EActivation::DontActivate);




      }

      void PhysicsUpdate(float dt) override
      {
            m_jolt->mPhysicsSystem->Update(dt, m_jolt->mCollisionSteps, m_jolt->mTempAllocator, m_jolt->mJobSystem);
            
      }
};

IGameMode* NewGameMode()
{
      return new GameMode{};
}
