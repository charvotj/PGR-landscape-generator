#pragma once

#include <iostream>
#include <fstream>
#include <geGL/geGL.h>
#include <geGL/StaticCalls.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "obj_load.h"
#include "gl_basics.h"
#include "camera_and_control.h"
#include "billboards.h"
#include "global_defines.h"
#include "shadows.h"

struct Tree {
    std::string name;
    GLuint instanceVBO;
    GLuint indirectBuffer;
    std::vector<InstanceData> instances;
    Object tree3D;
    Object treeBillboard;
};

void treesInit();
void setLodThreshold(GLfloat threshold);
void setFrustumCulling(bool state);
void loadTrees(std::vector<Material>& mats, std::vector<Tree>& trees);
void renderTrees(GLuint renderProgram, std::vector<Material>& mats, std::vector<Tree>& trees);