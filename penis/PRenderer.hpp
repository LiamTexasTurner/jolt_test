#pragma once
#include "renderer.hpp"
#include "shaderset.h"
#include "scene.hpp"
#include "preamble.glsl"
#include "glm/glm.hpp"


class PRenderer : public IRenderer
{
public:

      PRenderer() = default;

      Scene* m_scene;

      ShaderSet m_shaders;
      GLuint* m_scene_SP;
      GLuint* m_shadow_map_SP;
      GLuint* m_skybox_SP;
      GLuint* m_debug_depth_map_SP;
      GLuint* m_blit_texture_SP;
      GLuint* m_blit_test_SP;
      GLuint* m_PP_invert_color;
      GLuint* m_PP_crt;
      GLuint* m_PP_clear;
      GLuint* m_skinning;
      GLuint* m_skin_compute;
      
      unsigned int SCR_WIDTH;
      unsigned int SCR_HEIGHT;

      GLuint bone_transform_SSBO;
      GLuint inv_bind_pose_SSBO;
      GLuint anim_trs_SSBO;

      GLuint back_buffer_single_samp_FBO;
      GLuint back_buffer_single_samp_CT;
      GLuint back_buffer_single_samp_DT;
      
      
      GLuint back_buffer_multi_samp_FBO;
      GLuint back_buffer_multi_samp_CT;
      GLuint back_buffer_multi_samp_DT;
      int MSAA_SAMPLES = 16;

      GLuint post_buffer_FBO;
      GLuint post_buffer_CT;
      GLuint post_buffer_DT;

      GLuint out_buffer_FBO;
      GLuint out_buffer_CT;

      GLuint shadow_map_FBO;
      GLuint shadow_map_T;
      const unsigned int SHADOW_WIDTH = 4096 * 4, SHADOW_HEIGHT = 4096 * 4;
      bool render_shadow_map = false;

      GLuint skybox_cubemap_CT;

      GLuint m_null_vao;

      void DrawDebugQuad(){render_shadow_map = !render_shadow_map;}

      bool draw_skinned = false;
      void toggle_skinned(){draw_skinned = ! draw_skinned;}
      
      void Init(Scene* scene) override;
      void UpdateBuffers(Scene* scene) override;
      void Resize(int width, int height) override;
      unsigned int Paint() override;
      

      std::vector<uint32_t> shadow_draw_list{};
      std::vector<uint32_t> opaque_draw_list{};
      std::vector<uint32_t> transparent_draw_list{};
      std::vector<uint32_t> skinned_opaque_draw_list{};
      std::vector<uint32_t> skinned_mesh_instance_draw_list{};
      std::vector<uint32_t> skinned_transparent_draw_list{};
      void CreateDrawList();
      void DrawShadowMap(const glm::mat4& light_space_matrix);
      void DrawSkybox(const glm::mat4& projection, const glm::mat4& view);
      void DrawOpaque(const glm::mat4& projection,
                      const glm::mat4& view,
                      const glm::mat4& light_space_matrix,
                      const glm::vec3& light_pos,
                      const glm::vec3& dir_light_col);
      void DrawTransparent(const glm::mat4& projection,
                           const glm::mat4& view,
                           const glm::mat4& light_space_matrix,
                           const glm::vec3& light_pos,
                           const glm::vec3& dir_light_col);

      std::vector<TRS> all_poses;

      void DeformMeshGPU(const Skeleton* skeleton, std::span<const TRS> poses, GLuint* skinning_compute_shader);

      void DefromAllMeshesWithSkeleton();
      
      void BlitFrameBuffer(GLuint& read_buffer, GLuint draw_buffer, int width, int height);

      void PostProcess(GLuint& texture, GLuint* shader, int pos_x, int pos_y, int width, int height);

      void DrawTextureToQuad(GLuint& texture, GLuint& FBO, GLuint* shader, int pos_x, int pos_y, int width, int height);

      void BlitToOutBuffer(GLuint& texture, GLuint* shader, int pos_x, int pos_y, int width, int height);

      void DebugShadowMap(GLuint& shadow_map, int pos_x, int pos_y, int width, int height, float near_plane, float far_plane);

      
      void ResetRenderState();

};
