#pragma once

#include <geGL/geGL.h>
#include <geGL/StaticCalls.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "gl_basics.h"

extern GLuint depthMapProgram;
extern GLuint depthMapTexture;



void shadowMapInit();
void prepareDepthMapRender();
void programLightViewProj(GLuint program);
