#pragma once

class Scene;

class IRenderer
{
public:
      virtual void Init(Scene* scene) = 0;
      virtual void Resize(int width, int height) = 0;
      virtual unsigned int Paint() = 0;

      virtual void DrawDebugQuad() = 0;
};

IRenderer* NewRenderer();
