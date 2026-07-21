#include "PRenderer.hpp"

#include "GLFW/glfw3.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <glm/gtx/matrix_decompose.hpp>

#include "animation.hpp"

#include <iostream>
#include <string>
#include <algorithm>
#include <span>
#include <chrono>
#include <cstdint>

using namespace std;

void PRenderer::Init(Scene* scene) 
{
      m_scene = scene;
      

      m_shaders.SetVersion("440");
      m_shaders.SetPreambleFile("../penis/preamble.glsl");
      m_scene_SP = m_shaders.AddProgramFromExts({"../shaders/scene.vert", "../shaders/scene.frag"});
      m_shadow_map_SP = m_shaders.AddProgramFromExts({"../shaders/shadow_mapping_depth.vert", "../shaders/shadow_mapping_depth.frag"});
      m_skybox_SP = m_shaders.AddProgramFromExts({"../shaders/skybox.vert", "../shaders/skybox.frag"});
      m_debug_depth_map_SP = m_shaders.AddProgramFromExts({"../shaders/blit.vert", "../shaders/debug_depth_map.frag"});
      m_blit_texture_SP = m_shaders.AddProgramFromExts({"../shaders/blit.vert", "../shaders/blit_texture.frag"});
      m_blit_test_SP = m_shaders.AddProgramFromExts({"../shaders/blit.vert", "../shaders/blit_test.frag"});
      m_PP_invert_color = m_shaders.AddProgramFromExts({"../shaders/blit.vert", "../shaders/post_process_invert_color.frag"});
      m_PP_crt = m_shaders.AddProgramFromExts({"../shaders/blit.vert", "../shaders/post_process_crt.frag"});
      m_PP_clear = m_shaders.AddProgramFromExts({"../shaders/blit.vert", "../shaders/post_process_clear.frag"});
      m_skinning = m_shaders.AddProgramFromExts({"../shaders/skinning.vert","../shaders/skinning.frag"});
      m_skin_compute = m_shaders.AddProgramFromExts({"../shaders/skin.comp"});

      glGenVertexArrays(1, &m_null_vao);
      glBindVertexArray(m_null_vao);
      glBindVertexArray(0);

      all_poses.reserve(4096 * 4096);

}
void PRenderer::UpdateBuffers(Scene* scene)
{

      size_t buf_size = 4096 * 4096;
      vector<glm::mat4> inv_bind_mats;
      for(int i = 0; i < scene->skeletons.size(); i++)
      {
            Skeleton& skelton = scene->skeletons[i];
            inv_bind_mats.reserve(inv_bind_mats.size() + skelton.inv_bind_mats.size());
            inv_bind_mats.insert(inv_bind_mats.end(), skelton.inv_bind_mats.begin(), skelton.inv_bind_mats.end());
      }

      
      glGenBuffers(1, &inv_bind_pose_SSBO);
      glGenBuffers(1, &bone_transform_SSBO);
      glGenBuffers(1, &anim_trs_SSBO);

      {
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, inv_bind_pose_SSBO);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, inv_bind_pose_SSBO);
            GLsizeiptr buffer_size = sizeof(glm::mat4) * inv_bind_mats.size();
            glBufferData(GL_SHADER_STORAGE_BUFFER, buffer_size, inv_bind_mats.data(), GL_STATIC_DRAW);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

      }

      {
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, bone_transform_SSBO);
            GLsizeiptr buffer_size = sizeof(glm::mat4) * buf_size;
            glBufferData(GL_SHADER_STORAGE_BUFFER, buffer_size, nullptr, GL_DYNAMIC_DRAW);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);      
      }
      
      {
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, anim_trs_SSBO);
            GLsizeiptr buffer_size = sizeof(TRS) * buf_size;
            glBufferData(GL_SHADER_STORAGE_BUFFER, buffer_size, nullptr, GL_DYNAMIC_DRAW);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);      
      }
      
}

void PRenderer::Resize(int width, int height) 
{
      SCR_WIDTH = width;
      SCR_HEIGHT = height;
      //multi sample
      {
            glDeleteTextures(1, &back_buffer_multi_samp_CT);
            glGenTextures(1, &back_buffer_multi_samp_CT);
            glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, back_buffer_multi_samp_CT);
            glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, MSAA_SAMPLES, GL_RGB8, SCR_WIDTH, SCR_HEIGHT, GL_TRUE);
            glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

            glDeleteTextures(1, &back_buffer_multi_samp_DT);
            glGenTextures(1, &back_buffer_multi_samp_DT);
            glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, back_buffer_multi_samp_DT);
            glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, MSAA_SAMPLES, GL_DEPTH_COMPONENT24, SCR_WIDTH, SCR_HEIGHT, GL_TRUE);
            glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

            glDeleteFramebuffers(1, &back_buffer_multi_samp_FBO);
            glGenFramebuffers(1, &back_buffer_multi_samp_FBO);
            glBindFramebuffer(GL_FRAMEBUFFER, back_buffer_multi_samp_FBO);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, back_buffer_multi_samp_CT, 0);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, back_buffer_multi_samp_DT, 0);
            GLenum fbo_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
            if(fbo_status != GL_FRAMEBUFFER_COMPLETE)
            {
                  std::cout << "check fbo multi " << std::endl;
            }
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
      }


      //single sample
      {
            glDeleteTextures(1, &back_buffer_single_samp_CT);
            glGenTextures(1, &back_buffer_single_samp_CT);
            glBindTexture(GL_TEXTURE_2D, back_buffer_single_samp_CT);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glBindTexture(GL_TEXTURE_2D, 0);


            glDeleteFramebuffers(1, &back_buffer_single_samp_FBO);
            glGenFramebuffers(1, &back_buffer_single_samp_FBO);
            glBindFramebuffer(GL_FRAMEBUFFER, back_buffer_single_samp_FBO);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D, back_buffer_single_samp_CT, 0);

            GLenum single_samp_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
            if(single_samp_status != GL_FRAMEBUFFER_COMPLETE)
            {
                  std::cout << "check fbo single " << std::endl;
            }

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
      }

      // shadow maps
      {

            glDeleteFramebuffers(1, &shadow_map_FBO);
            glGenFramebuffers(1, &shadow_map_FBO);

            glDeleteTextures(1, &shadow_map_T);
            glGenTextures(1, &shadow_map_T);
            
            glBindTexture(GL_TEXTURE_2D, shadow_map_T);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
            glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);  
            // attach depth texture as FBO's depth buffer
            glBindFramebuffer(GL_FRAMEBUFFER, shadow_map_FBO);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadow_map_T, 0);
            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
      }

      //post process
      {

            glDeleteTextures(1, &post_buffer_CT);
            glGenTextures(1, &post_buffer_CT);
            glBindTexture(GL_TEXTURE_2D, post_buffer_CT);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glBindTexture(GL_TEXTURE_2D, 0);

            glDeleteTextures(1, &post_buffer_DT);
            glGenTextures(1, &post_buffer_DT);
            glBindTexture(GL_TEXTURE_2D, post_buffer_DT);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
            glBindTexture(GL_TEXTURE_2D, 0);



            glDeleteFramebuffers(1, &post_buffer_FBO);
            glGenFramebuffers(1, &post_buffer_FBO);
            glBindFramebuffer(GL_FRAMEBUFFER, post_buffer_FBO);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D, post_buffer_CT, 0);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, post_buffer_DT, 0);

            GLenum single_samp_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
            if(single_samp_status != GL_FRAMEBUFFER_COMPLETE)
            {
                  std::cout << "check fbo single " << std::endl;
            }

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
      }

      //out buffer
      {
            glDeleteTextures(1, &out_buffer_CT);
            glGenTextures(1, &out_buffer_CT);
            glBindTexture(GL_TEXTURE_2D, out_buffer_CT);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glBindTexture(GL_TEXTURE_2D, 0);


            glDeleteFramebuffers(1, &out_buffer_FBO);
            glGenFramebuffers(1, &out_buffer_FBO);
            glBindFramebuffer(GL_FRAMEBUFFER, out_buffer_FBO);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D, out_buffer_CT, 0);

            GLenum single_samp_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
            if(single_samp_status != GL_FRAMEBUFFER_COMPLETE)
            {
                  std::cout << "check fbo single " << std::endl;
            }

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
      }
      
      
}

unsigned int PRenderer::Paint() 
{
      m_shaders.UpdatePrograms();
                        
                              
      glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 3000.0f);
                  
      Camera* cam = &m_scene->cameras[m_scene->main_camera_ID];

      glm::mat4 view = glm::inverse(cam->GetWorldMat());

      float near_plane = -10.0f, far_plane = 20.5f;
      float ortho_size = 20.0f;
      glm::vec3 light_pos = glm::vec3(-1.0f, 2.0f, 0.2f);
      
      glm::mat4 light_projection = glm::ortho(-ortho_size, ortho_size, -ortho_size, ortho_size, near_plane, far_plane);
      glm::mat4 light_view = glm::lookAt(light_pos, 
                                         glm::vec3(0.0f), 
                                         glm::vec3( 0.0f, 1.0f,  0.0f));
      glm::mat4 light_space_matrix = light_projection * light_view;

      float intensity = 10.0f;
      
      glm::vec3 dir_light_col = glm::vec3(78.0f + intensity,
                                          77.0f + intensity,
                                          92.0f + intensity) / 100.0f;


      CreateDrawList();
      
      DrawShadowMap(light_space_matrix);
      
      DrawSkybox(projection, view);

      DrawOpaque(projection, view, light_space_matrix, light_pos, dir_light_col);
      
      DrawTransparent(projection, view, light_space_matrix, light_pos, dir_light_col);

      BlitFrameBuffer(back_buffer_multi_samp_FBO, back_buffer_single_samp_FBO, SCR_WIDTH, SCR_HEIGHT);
      
      PostProcess(back_buffer_single_samp_CT, m_PP_clear, 0, 0, SCR_WIDTH, SCR_HEIGHT);

      DrawTextureToQuad(post_buffer_CT, out_buffer_FBO, m_blit_texture_SP, 0, 0, SCR_WIDTH, SCR_HEIGHT);

      if(render_shadow_map)
      {
            DebugShadowMap(shadow_map_T, 0, SCR_HEIGHT - SCR_HEIGHT / 3, SCR_WIDTH / 3, SCR_HEIGHT / 3, near_plane, far_plane);
      }

      return out_buffer_CT;
}

void PRenderer::CreateDrawList()
{
      opaque_draw_list.clear();
      transparent_draw_list.clear();
      skinned_opaque_draw_list.clear();
      skinned_transparent_draw_list.clear();
      skinned_mesh_instance_draw_list.clear();
      for(int instance_ID = 0; instance_ID < m_scene->instances.size(); instance_ID++)
      {
            const Instance& instance = m_scene->instances[instance_ID];
            
            if(m_scene->meshes.contains(instance.mesh_ID))
            {
                  const Mesh& mesh = m_scene->meshes[instance.mesh_ID];
                  for(int mesh_draw_index = 0; mesh_draw_index < mesh.draw_commands.size(); mesh_draw_index++)
                  {
                        if(mesh.material_IDs.size() > 0)
                        {
                              const Material& material = m_scene->materials[mesh.material_IDs[mesh_draw_index]];
                              const DiffuseMap& diffuse_map = m_scene->diffuse_maps[material.diffuse_map_ID];
                              if(diffuse_map.has_transparency)
                              {
                                    transparent_draw_list.push_back(instance_ID);
                              }
                              else
                              {
                                    opaque_draw_list.push_back(instance_ID);
                              }
                        }
                        
                  }
            }
            if(m_scene->skinned_meshes.contains(instance.skinned_mesh_ID))
            {
                  skinned_mesh_instance_draw_list.push_back(instance_ID);
                  const SkinnedMesh& skinned_mesh = m_scene->skinned_meshes[instance.skinned_mesh_ID];
                  for(int skinned_mesh_draw_index = 0; skinned_mesh_draw_index < skinned_mesh.draw_commands.size(); skinned_mesh_draw_index++)
                  {
                        const Material& material = m_scene->materials[skinned_mesh.material_IDs[skinned_mesh_draw_index]];
                        const DiffuseMap& diffuse_map = m_scene->diffuse_maps[material.diffuse_map_ID];
                        if(diffuse_map.has_transparency)
                        {
                              skinned_transparent_draw_list.push_back(instance_ID);
                        }
                        else
                        {
                              skinned_opaque_draw_list.push_back(instance_ID);
                        }
                  }
            }
      }
      glm::vec3 main_cam_pos = m_scene->cameras[m_scene->main_camera_ID].translation;
      sort(transparent_draw_list.begin(), transparent_draw_list.end(),
           [main_cam_pos, this](const uint32_t& a, const uint32_t& b)
                 {      
                       float dist_a = glm::length(main_cam_pos - m_scene->transforms[m_scene->instances[a].transform_ID].translation);
                       float dist_b = glm::length(main_cam_pos - m_scene->transforms[m_scene->instances[b].transform_ID].translation);
                       return dist_a < dist_b;
                 });           
}

void PRenderer::DrawShadowMap(const glm::mat4& light_space_matrix)
{
      glBindFramebuffer(GL_FRAMEBUFFER, shadow_map_FBO);
      glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
      glEnable(GL_DEPTH_TEST);     
      glClear(GL_DEPTH_BUFFER_BIT);
      glUseProgram(*m_shadow_map_SP);
      
      for(uint32_t instance_ID : shadow_draw_list)
      {
            const Instance* instance = &m_scene->instances[instance_ID];
            const Mesh* mesh = &m_scene->meshes[instance->mesh_ID];
            Transform* transform = &m_scene->transforms[instance->transform_ID];
            transform->scale = glm::vec3(1.0f);

            glm::mat4 MW = glm::mat4(1.0f);
            MW = translate(-transform->rotation_origin) * MW;
            MW = mat4_cast(transform->rotation) * MW;
            MW = translate(transform->rotation_origin) * MW;
            MW = scale(transform->scale) * MW;
            MW = translate(transform->translation) * MW;

            glUniformMatrix4fv(SHADOW_MAP_MW_UNIFORM_LOCATION, 1, GL_FALSE, value_ptr(MW));
            glUniformMatrix4fv(SHADOW_MAP_LIGHT_SPACE_MATRIX_UNIFORM_LOCATION, 1, GL_FALSE, value_ptr(light_space_matrix));

            glBindVertexArray(mesh->mesh_VAO);
            for(size_t mesh_draw_index = 0; mesh_draw_index < mesh->draw_commands.size(); mesh_draw_index++)
            {
                  const DrawCommand* draw_cmd = &mesh->draw_commands[mesh_draw_index];

                  const Material* material = &m_scene->materials[mesh->material_IDs[mesh_draw_index]];

                  glActiveTexture(GL_TEXTURE0 + SHADOW_DIFFUSE_MAP_TEXTURE_BINDING);
                  const DiffuseMap* diffuse_map = &m_scene->diffuse_maps[material->diffuse_map_ID];
                  glBindTexture(GL_TEXTURE_2D, diffuse_map->DiffuseMapTO);

                  glDrawElementsInstancedBaseVertexBaseInstance(GL_TRIANGLES,
                                                                draw_cmd->count,
                                                                GL_UNSIGNED_INT,
                                                                (GLvoid*)(sizeof(uint32_t) * draw_cmd->firstIndex),
                                                                draw_cmd->primCount,
                                                                draw_cmd->baseVertex,
                                                                draw_cmd->baseInstance);
            }                  
            glBindVertexArray(0);
      }
      ResetRenderState();
}

void PRenderer::DrawSkybox(const glm::mat4& projection,
                           const glm::mat4& view)
{
      glBindFramebuffer(GL_FRAMEBUFFER, back_buffer_multi_samp_FBO);
      glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
      glEnable(GL_DEPTH_TEST);
      glDepthMask(GL_TRUE);
      glDepthFunc(GL_LEQUAL);      

      glUseProgram(*m_skybox_SP);
      glm::mat4 skybox_view = glm::mat4(glm::mat3(view));
      glUniformMatrix4fv(SKYBOX_PROJECTION_UNIFORM_LOCATION, 1, GL_FALSE, value_ptr(projection));
      glUniformMatrix4fv(SKYBOX_VIEW_UNIFORM_LOCATION, 1, GL_FALSE, value_ptr(skybox_view));
      Skybox* skybox = &m_scene->skyboxes[0];
      glBindVertexArray(skybox->skyboxVAO);
      glActiveTexture(GL_TEXTURE0 + SKYBOX_TEXTURE_BINDING);
      glBindTexture(GL_TEXTURE_CUBE_MAP, skybox->cube_map_texture);
      glDrawArrays(GL_TRIANGLES, 0, 36);
      ResetRenderState();
}

void PRenderer::DrawOpaque(const glm::mat4& projection,
                           const glm::mat4& view,
                           const glm::mat4& light_space_matrix,
                           const glm::vec3& light_pos,
                           const glm::vec3& dir_light_col)
{
      glBindFramebuffer(GL_FRAMEBUFFER, back_buffer_multi_samp_FBO);
      glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
      glEnable(GL_DEPTH_TEST);
      glDepthMask(GL_TRUE);
      glDepthFunc(GL_LESS);
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      glEnable(GL_CULL_FACE);      
      glCullFace(GL_BACK);
      glFrontFace(GL_CCW);
      glEnable(GL_DEPTH_TEST);

      
      glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
      glm::mat4 VP = projection * view;
      glUseProgram(*m_scene_SP);
      glUniform3fv(SCENE_CAMERAPOS_UNIFORM_LOCATION, 1, glm::value_ptr(view[3]));
      
      

      for(uint32_t instance_ID : opaque_draw_list)
      {
            const Instance* instance = &m_scene->instances[instance_ID];
            const Mesh* mesh = &m_scene->meshes[instance->mesh_ID];
            Transform* transform = &m_scene->transforms[instance->transform_ID];
            transform->scale = glm::vec3(1.0f);

            glm::mat4 MW = glm::mat4(1.0f);
            MW = translate(-transform->rotation_origin) * MW;
            MW = mat4_cast(transform->rotation) * MW;
            MW = translate(transform->rotation_origin) * MW;
            MW = scale(transform->scale) * MW;
            MW = translate(transform->translation) * MW;

            glm::mat4 MVP = VP * MW;


            glm::mat3 N_MW = glm::mat4(1.0f);
            N_MW = mat3_cast(transform->rotation) * N_MW;
            N_MW = glm::mat3(scale(1.0f / transform->scale)) * N_MW;

            glUniformMatrix4fv(SCENE_MW_UNIFORM_LOCATION, 1, GL_FALSE, value_ptr(MW));
            glUniformMatrix4fv(SCENE_N_MW_UNIFORM_LOCATION, 1, GL_FALSE, value_ptr(N_MW));
            glUniformMatrix4fv(SCENE_MVP_UNIFORM_LOCATION, 1, GL_FALSE, value_ptr(MVP));
            glUniformMatrix4fv(SCENE_VIEW_UNIFORM_LOCATION, 1, GL_FALSE, value_ptr(view));
            glUniformMatrix4fv(SCENE_PROJECTON_UNIFORM_LOCATION, 1, GL_FALSE, value_ptr(projection));
            glUniformMatrix4fv(SCENE_LIGHT_SPACE_MATRIX_UNIFORM_LOCATION, 1, GL_FALSE, value_ptr(light_space_matrix));
            glUniform3fv(SCENE_LIGHT_POS, 1, value_ptr(light_pos));
            glUniform3fv(SCENE_LIGHT_COLOR, 1, value_ptr(dir_light_col));

            glActiveTexture(GL_TEXTURE0 + SCENE_SHADOW_MAP_TEXTURE_BINDING);
            glBindTexture(GL_TEXTURE_2D, shadow_map_T);
            glBindVertexArray(mesh->mesh_VAO);
            for(size_t mesh_draw_index = 0; mesh_draw_index < mesh->draw_commands.size(); mesh_draw_index++)
            {
                  const DrawCommand* draw_cmd = &mesh->draw_commands[mesh_draw_index];
                  const Material* material = &m_scene->materials[mesh->material_IDs[mesh_draw_index]];

                  if(material->scale.x != 1)
                  {
                        float x = 0;
                  }


                  glUniform2fv(SCENE_TEXCOORD_SCALE, 1, value_ptr(material->scale));
                  glUniform2fv(SCENE_TEXCOORD_OFFSET, 1, value_ptr(material->offset));

                  glActiveTexture(GL_TEXTURE0 + SCENE_DIFFUSE_MAP_TEXTURE_BINDING);
                  if(!material->has_diffuse_map)
                  {
                        glBindTexture(GL_TEXTURE_2D, 0);
                        glUniform1i(SCENE_HAS_DIFFUSE_MAP_UNIFORM_LOCATION, 0);
                  }
                  else
                  {
                        const DiffuseMap* diffuse_map = &m_scene->diffuse_maps[material->diffuse_map_ID];
                        glBindTexture(GL_TEXTURE_2D, diffuse_map->DiffuseMapTO);
                        glUniform1i(SCENE_HAS_DIFFUSE_MAP_UNIFORM_LOCATION, 1);
                  }

                  glDrawElementsInstancedBaseVertexBaseInstance(GL_TRIANGLES,
                                                                draw_cmd->count,
                                                                GL_UNSIGNED_INT,
                                                                (GLvoid*)(sizeof(uint32_t) * draw_cmd->firstIndex),

                                                                draw_cmd->primCount,
                                                                draw_cmd->baseVertex,
                                                                draw_cmd->baseInstance);


            }

            glBindVertexArray(0);
      }
      

      //draw skinned mesh
      DefromAllMeshesWithSkeleton();
      glUseProgram(*m_skinning);
      glUniformMatrix4fv(SCENE_VIEW_UNIFORM_LOCATION, 1, GL_FALSE, value_ptr(view));
      glUniformMatrix4fv(SCENE_PROJECTON_UNIFORM_LOCATION, 1, GL_FALSE, value_ptr(projection));
      glActiveTexture(GL_TEXTURE0 + SCENE_DIFFUSE_MAP_TEXTURE_BINDING);
      glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SCENE_BONE_MAT_SSBO_BINDING, bone_transform_SSBO);
      for(int i = 0; i < skinned_opaque_draw_list.size(); i++)
      {
            uint32_t instance_ID = skinned_opaque_draw_list[i];
            const Instance* instance = &m_scene->instances[instance_ID];
            const SkinnedMesh* skinned_mesh = &m_scene->skinned_meshes[instance->skinned_mesh_ID];
            const Skeleton* skeleton = &m_scene->skeletons[skinned_mesh->skeleton_ID];
            const AnimationGraph* anim_graph = &m_scene->animation_graphs[skinned_mesh->anim_graph_ID];

            // FK(skeleton->bone_info, anim_graph->out_pose);
            
            // DeformMeshGPU(skeleton, anim_graph->out_pose, m_skin_compute);            
            
            Transform* transform = &m_scene->transforms[instance->transform_ID];
            transform->scale = glm::vec3(1.0f);

            glm::mat4 MW = glm::mat4(1.0f);
            MW = translate(-transform->rotation_origin) * MW;
            MW = mat4_cast(transform->rotation) * MW;
            MW = translate(transform->rotation_origin) * MW;
            MW = scale(transform->scale) * MW;
            MW = translate(transform->translation) * MW;

            // glm::mat4 MW = glm::translate(glm::mat4(1.0f), glm::vec3(i, 0.0f, 0.0f));

            

            
            
            glUniformMatrix4fv(SCENE_MW_UNIFORM_LOCATION, 1, GL_FALSE, value_ptr(MW));
            glUniform1i(SCENE_BONE_OFFSET_UNIFORM_LOCATION, skeleton->bone_count * i);
            
            glBindVertexArray(skinned_mesh->mesh_VAO);
            for(size_t mesh_draw_index = 0; mesh_draw_index < skinned_mesh->draw_commands.size(); mesh_draw_index++)
            {
                  const DrawCommand* draw_cmd = &skinned_mesh->draw_commands[mesh_draw_index];
                  const Material* material = &m_scene->materials[skinned_mesh->material_IDs[mesh_draw_index]];


                  glUniform2fv(SCENE_TEXCOORD_SCALE, 1, value_ptr(material->scale));
                  glUniform2fv(SCENE_TEXCOORD_OFFSET, 1, value_ptr(material->offset));

                  
                  if(material->diffuse_map_ID == -1)
                  {
                        glBindTexture(GL_TEXTURE_2D, 0);
                        glUniform1i(SCENE_HAS_DIFFUSE_MAP_UNIFORM_LOCATION, 0);
                  }
                  else
                  {
                        const DiffuseMap* diffuse_map = &m_scene->diffuse_maps[material->diffuse_map_ID];
                        glBindTexture(GL_TEXTURE_2D, diffuse_map->DiffuseMapTO);
                        glUniform1i(SCENE_HAS_DIFFUSE_MAP_UNIFORM_LOCATION, 1);
                  }

                  glDrawElementsInstancedBaseVertexBaseInstance(GL_TRIANGLES,
                                                                draw_cmd->count,
                                                                GL_UNSIGNED_INT,
                                                                (GLvoid*)(sizeof(uint32_t) * draw_cmd->firstIndex),
                                                                draw_cmd->primCount,
                                                                draw_cmd->baseVertex,
                                                                draw_cmd->baseInstance);
            }

      }
      glUseProgram(0);
      glBindVertexArray(0);
      
      ResetRenderState();
}

void PRenderer::DrawTransparent(const glm::mat4& projection,
                                const glm::mat4& view,
                                const glm::mat4& light_space_matrix,
                                const glm::vec3& light_pos,
                                const glm::vec3& dir_light_col)
{
      glBindFramebuffer(GL_FRAMEBUFFER, back_buffer_multi_samp_FBO);
      glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
      glEnable(GL_DEPTH_TEST);
      glDepthMask(GL_TRUE);
      glDepthFunc(GL_LESS);
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      
      glEnable(GL_DEPTH_TEST);

      glUseProgram(*m_scene_SP);
      glUniform3fv(SCENE_CAMERAPOS_UNIFORM_LOCATION, 1, glm::value_ptr(view[3]));
      glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

      glm::mat4 VP = projection * view;
      
      for(uint32_t instance_ID : transparent_draw_list)
      {
            const Instance* instance = &m_scene->instances[instance_ID];
            const Mesh* mesh = &m_scene->meshes[instance->mesh_ID];
            Transform* transform = &m_scene->transforms[instance->transform_ID];
            transform->scale = glm::vec3(1.0f);

            glm::mat4 MW = glm::mat4(1.0f);
            MW = translate(-transform->rotation_origin) * MW;
            MW = mat4_cast(transform->rotation) * MW;
            MW = translate(transform->rotation_origin) * MW;
            MW = scale(transform->scale) * MW;
            MW = translate(transform->translation) * MW;

            glm::mat4 MVP = VP * MW;


            glm::mat3 N_MW = glm::mat4(1.0f);
            N_MW = mat3_cast(transform->rotation) * N_MW;
            N_MW = glm::mat3(scale(1.0f / transform->scale)) * N_MW;

            glUniformMatrix4fv(SCENE_MW_UNIFORM_LOCATION, 1, GL_FALSE, value_ptr(MW));
            glUniformMatrix4fv(SCENE_N_MW_UNIFORM_LOCATION, 1, GL_FALSE, value_ptr(N_MW));
            glUniformMatrix4fv(SCENE_MVP_UNIFORM_LOCATION, 1, GL_FALSE, value_ptr(MVP));
            glUniformMatrix4fv(SCENE_LIGHT_SPACE_MATRIX_UNIFORM_LOCATION, 1, GL_FALSE, value_ptr(light_space_matrix));
            glUniform3fv(SCENE_LIGHT_POS, 1, value_ptr(light_pos));
            glUniform3fv(SCENE_LIGHT_COLOR, 1, value_ptr(dir_light_col));

            glActiveTexture(GL_TEXTURE0 + SCENE_SHADOW_MAP_TEXTURE_BINDING);
            glBindTexture(GL_TEXTURE_2D, shadow_map_T);
            glBindVertexArray(mesh->mesh_VAO);
            for(size_t mesh_draw_index = 0; mesh_draw_index < mesh->draw_commands.size(); mesh_draw_index++)
            {
                  if(mesh->draw_commands[mesh_draw_index].has_transparecny)
                  {
                        float x = 0;
                  }
                  const DrawCommand* draw_cmd = &mesh->draw_commands[mesh_draw_index];
                  const Material* material = &m_scene->materials[mesh->material_IDs[mesh_draw_index]];

                  glActiveTexture(GL_TEXTURE0 + SCENE_DIFFUSE_MAP_TEXTURE_BINDING);
                  if(!material->has_diffuse_map)
                  {
                        glBindTexture(GL_TEXTURE_2D, 0);
                        glUniform1i(SCENE_HAS_DIFFUSE_MAP_UNIFORM_LOCATION, 0);
                  }
                  else
                  {
                        const DiffuseMap* diffuse_map = &m_scene->diffuse_maps[material->diffuse_map_ID];
                        glBindTexture(GL_TEXTURE_2D, diffuse_map->DiffuseMapTO);
                        glUniform1i(SCENE_HAS_DIFFUSE_MAP_UNIFORM_LOCATION, 1);
                  }

                  glDrawElementsInstancedBaseVertexBaseInstance(GL_TRIANGLES,
                                                                draw_cmd->count,
                                                                GL_UNSIGNED_INT,
                                                                (GLvoid*)(sizeof(uint32_t) * draw_cmd->firstIndex),
                                                                draw_cmd->primCount,
                                                                draw_cmd->baseVertex,
                                                                draw_cmd->baseInstance);
            }
            glBindVertexArray(0);
      }
      ResetRenderState();

}

void PRenderer::DefromAllMeshesWithSkeleton()
{

      glUseProgram(*m_skin_compute);
      
      all_poses.clear();
      for(int i = 0; i < skinned_mesh_instance_draw_list.size(); i++)
      {
            uint32_t instance_ID = skinned_mesh_instance_draw_list[i];
            const Instance* instance = &m_scene->instances[instance_ID];
            const AnimationGraph* anim_graph = &m_scene->animation_graphs[instance->anim_graph_ID];      
            all_poses.insert(all_poses.end(), anim_graph->out_pose.begin(), anim_graph->out_pose.end());
      }


      unsigned int bone_count = 92;
      int instances = skinned_opaque_draw_list.size();
      glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SKINNING_COMPUTE_INV_BIND_POSE_BINDING, inv_bind_pose_SSBO);
      glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SCENE_BONE_MAT_SSBO_BINDING, bone_transform_SSBO);
      
      GLsizeiptr buffer_size = sizeof(TRS) * (bone_count * instances);
      glBindBuffer(GL_SHADER_STORAGE_BUFFER, anim_trs_SSBO);
      glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, buffer_size, all_poses.data());
      glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SKINNING_COMPUTE_TRS_BINDING, anim_trs_SSBO);      

      
      glUniform1ui(SKINNING_COMP_BONE_COUNT_UNIFORM_LOCATION, bone_count);
      glDispatchCompute((bone_count + SKINNING_GROUP_SIZE_X - 1) / SKINNING_GROUP_SIZE_X, instances, 1);
      glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void PRenderer::DeformMeshGPU(const Skeleton* skeleton, std::span<const TRS> poses, GLuint* skinning_compute_shader)
{


      glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SKINNING_COMPUTE_INV_BIND_POSE_BINDING, inv_bind_pose_SSBO);
      glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SCENE_BONE_MAT_SSBO_BINDING, bone_transform_SSBO);
      
      GLsizeiptr buffer_size = sizeof(TRS) * skeleton->bone_count;
      glBindBuffer(GL_SHADER_STORAGE_BUFFER, anim_trs_SSBO);
      glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, buffer_size, poses.data());
      glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SKINNING_COMPUTE_TRS_BINDING, anim_trs_SSBO);      
      
      glUseProgram(*skinning_compute_shader);
      glDispatchCompute((skeleton->bone_count + SKINNING_GROUP_SIZE_X - 1) / SKINNING_GROUP_SIZE_X, 1, 1);
      glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void PRenderer::BlitFrameBuffer(GLuint& read_buffer, GLuint draw_buffer, int width, int height)
{
      glBindFramebuffer(GL_READ_FRAMEBUFFER, read_buffer);
      glBindFramebuffer(GL_DRAW_FRAMEBUFFER, draw_buffer);

      glBlitFramebuffer(0, 0, width, height,
                        0, 0, width, height,
                        GL_COLOR_BUFFER_BIT,
                        GL_NEAREST);
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PRenderer::PostProcess(GLuint& texture, GLuint* shader, int pos_x, int pos_y, int width, int height)
{
      glBindFramebuffer(GL_FRAMEBUFFER, post_buffer_FBO);
      glViewport(pos_x, pos_y, width, height);
      
      glDisable(GL_DEPTH_TEST);
      glDisable(GL_CULL_FACE);
      glDepthMask(GL_FALSE);
      glClear(GL_COLOR_BUFFER_BIT);
      
      glUseProgram(*shader);
      glUniform1f(POST_PROCESS_ITIME_UNIFORM_LOCATION, glfwGetTime());
      
      glActiveTexture(GL_TEXTURE0 + POST_PROCESS_TEXTURE_BINDING);
      glBindTexture(GL_TEXTURE_2D, texture);
      
      glBindVertexArray(m_null_vao);
      glDrawArrays(GL_TRIANGLES, 0, 3);
      
      ResetRenderState();      
}

void PRenderer::DrawTextureToQuad(GLuint& texture, GLuint& FBO, GLuint* shader, int pos_x, int pos_y, int width, int height)
{
      glBindFramebuffer(GL_FRAMEBUFFER, FBO);
      glViewport(pos_x, pos_y, width, height);
      glDisable(GL_DEPTH_TEST);
      glDisable(GL_CULL_FACE);
      glDepthMask(GL_FALSE);
      glClear(GL_COLOR_BUFFER_BIT);
      glUseProgram(*shader);
      glActiveTexture(GL_TEXTURE0 + BLIT_TEXTURE_TEXURE_BINDING);
      glBindTexture(GL_TEXTURE_2D, texture);
      glBindVertexArray(m_null_vao);
      glDrawArrays(GL_TRIANGLES, 0, 3);
      ResetRenderState();
}

void PRenderer::DebugShadowMap(GLuint& shadow_map, int pos_x, int pos_y, int width, int height, float near_plane, float far_plane)
{
      glBindFramebuffer(GL_FRAMEBUFFER, out_buffer_FBO);
      glDisable(GL_CULL_FACE);
      glViewport(pos_x, pos_y, width, height);
      glUseProgram(*m_debug_depth_map_SP);
      glBindVertexArray(m_null_vao);
      glActiveTexture(GL_TEXTURE0 + DEBUG_DEPTH_MAP_TEXURE_BINDING);
      glBindTexture(GL_TEXTURE_2D, shadow_map);
      glUniform1f(DEBUG_DEPTH_MAP_NEAR_PLANE, near_plane);
      glUniform1f(DEBUG_DEPTH_MAP_FAR_PLANE, far_plane);
      glDrawArrays(GL_TRIANGLES, 0, 3);
      ResetRenderState();
}


void PRenderer::ResetRenderState()
{
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      glBindVertexArray(0);
      glUseProgram(0);

      glDisable(GL_DEPTH_TEST);
      glDisable(GL_CULL_FACE);
      glDisable(GL_BLEND);

      glDepthMask(GL_TRUE);
      glDepthFunc(GL_LESS);

      glActiveTexture(GL_TEXTURE0);
}
