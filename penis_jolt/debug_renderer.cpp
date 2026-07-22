#include "debug_renderer.hpp"

#include <iostream>



pDebugRenderer::pDebugRenderer()
{
      DebugRenderer::Initialize();
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

      glGenVertexArrays(1, &prim->VAO);
      glGenBuffers(1, &prim->VBO);

	return prim;
}


void pDebugRenderer::DrawGeometry(RMat44Arg inModelMatrix,
                                  const AABox &inWorldSpaceBounds,
                                  float inLODScaleSq,
                                  ColorArg inModelColor,
                                  const GeometryRef &inGeometry,
                                  ECullMode inCullMode, ECastShadow inCastShadow, EDrawMode inDrawMode) 
{
      float x = 0;
}



void pDebugRenderer::DrawText3D(RVec3Arg inPosition,
                                const string_view &inString,
                                ColorArg inColor,
                                float inHeight)
{
      float x = 0;
}


