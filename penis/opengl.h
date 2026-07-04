#pragma once
#include <glad/glad.h>
// According to glDrawElementsIndirect documentation
typedef struct GLDrawElementsIndirectCommand
{
    GLuint count;
    GLuint primCount;
    GLuint firstIndex;
    GLint baseVertex;
    GLuint baseInstance;
} GLDrawElementsIndirectCommand;
