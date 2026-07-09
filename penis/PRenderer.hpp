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

      unsigned int SCR_WIDTH;
      unsigned int SCR_HEIGHT;

      GLuint back_buffer_single_samp_FBO;
      GLuint back_buffer_single_samp_CT;
      GLuint back_buffer_single_samp_DT;
      
      
      GLuint back_buffer_multi_samp_FBO;
      GLuint back_buffer_multi_samp_CT;
      GLuint back_buffer_multi_samp_DT;
      int MSAA_SAMPLES = 16;

      GLuint shadow_map_FBO;
      GLuint shadow_map_T;
      const unsigned int SHADOW_WIDTH = 4096 * 4, SHADOW_HEIGHT = 4096 * 4;
      bool render_shadow_map = false;

      GLuint skybox_cubemap_CT;

      GLuint m_null_vao;

      void DrawDebugQuad(){render_shadow_map = !render_shadow_map;}

      void Init(Scene* scene) override;
      void Resize(int width, int height) override;
      void Paint() override;

      std::vector<uint32_t> opaque_draw_list;
      std::vector<uint32_t> transparent_draw_list;
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
      
      void BlitFrameBuffer(GLuint& read_buffer, GLuint draw_buffer, int width, int height);

      void DrawTextureToQuad(GLuint& texture, int pos_x, int pos_y, int width, int height);

      void DebugShadowMap(GLuint& shadow_map, int pos_x, int pos_y, int width, int height, float near_plane, float far_plane);

      
      void ResetRenderState();

      

};
