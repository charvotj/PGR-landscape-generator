#pragma once

#include <SDL3/SDL.h>
#include <geGL/geGL.h>
#include <geGL/StaticCalls.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "gl_basics.h"

bool handleControls();
void updateViewProj(GLuint program, bool viewProjTogether = true);
void camera_init();

glm::vec3 getCameraPosition();
glm::mat4 getCameraViewProj();
bool getCameraChangeStateForLod();
bool getCameraViewProjChange();