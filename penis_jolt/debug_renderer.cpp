#include "debug_renderer.hpp"
#include "../penis/preamble.glsl"
#include <iostream>
#include <glm/matrix.hpp>
#include <glm/gtc/type_ptr.hpp>



pDebugRenderer::pDebugRenderer()
{
      DebugRenderer::Initialize();
      debug_line_SP.ShaderInit("../shaders/debug_line.vert", "../shaders/debug_line.frag");
}
            
void pDebugRenderer::DrawLine(RVec3Arg inFrom, RVec3Arg inTo, ColorArg inColor) 
{
      float x = 0;
}

void pDebugRenderer::DrawTriangle(RVec3Arg inV1, RVec3Arg inV2, RVec3Arg inV3, ColorArg inColor, ECastShadow inCastShadow) 
{
      float x = 0;
}


DebugRenderer::Batch pDebugRenderer::CreateTriangleBatch(const Triangle *inTriangles, int inTriangleCount) 
{
      DebugRenderer::Batch batch;
      return batch;
}


DebugRenderer::Batch pDebugRenderer::CreateTriangleBatch(const Vertex *inVertices, int inVertexCount, const uint32 *inIndices, int inIndexCount) 
{
      pRenderPrimitive* prim = new pRenderPrimitive{};

      prim->vertex_count = inIndexCount;

      glGenVertexArrays(1, &prim->VAO);
      glGenBuffers(1, &prim->VBO);
      glGenBuffers(1, &prim->EBO);

      glBindVertexArray(prim->VAO);
      
      glBindBuffer(GL_ARRAY_BUFFER, prim->VBO);
      glBufferData(GL_ARRAY_BUFFER, inVertexCount, inVertices, GL_STATIC_DRAW);

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, prim->EBO);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, inIndexCount, inIndices, GL_STATIC_DRAW);

      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
      glEnableVertexAttribArray(0);

	return prim;
}


void pDebugRenderer::DrawGeometry(RMat44Arg inModelMatrix,
                                  const AABox &inWorldSpaceBounds,
                                  float inLODScaleSq,
                                  ColorArg inModelColor,
                                  const GeometryRef &inGeometry,
                                  ECullMode inCullMode, ECastShadow inCastShadow, EDrawMode inDrawMode) 
{
      
      pRenderPrimitive* prim = dynamic_cast<pRenderPrimitive*>(inGeometry.GetPtr()->mLODs[0].mTriangleBatch.GetPtr());{}
      GLuint VAO = prim->VAO;
      GLuint VBO = prim->VBO;

      
      debug_line_SP.use();
      
      
      glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glDisable(GL_DEPTH_TEST);
      
      
      glBindVertexArray(VAO);
      glBindBuffer(GL_ARRAY_BUFFER, VBO);
      
      debug_line_SP.setMat4("model", glm::mat4(1.0f));
      debug_line_SP.setMat4("view", view);
      debug_line_SP.setMat4("projection", projection);    

      glDrawArrays(GL_LINES, 0, (GLsizei)prim->vertex_count);

      
      
}



void pDebugRenderer::DrawText3D(RVec3Arg inPosition,
                                const string_view &inString,
                                ColorArg inColor,
                                float inHeight)
{
      float x = 0;
}


