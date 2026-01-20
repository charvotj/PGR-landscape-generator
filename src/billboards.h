#pragma once

#include <SDL3/SDL.h>
#include <geGL/geGL.h>
#include <geGL/StaticCalls.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "gl_basics.h"
#include "skybox.h"
#include "camera_and_control.h"

void addBillboardTrees(std::vector<Material>& mats, std::vector<Object>& treeObjects, std::vector<Object>& treeBillboards);