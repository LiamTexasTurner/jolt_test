#include "renderer.hpp"
#include "PRenderer.hpp"
      
IRenderer* NewRenderer()
{
      return new PRenderer();
}
