#pragma once

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <unordered_map>
#include <geGL/geGL.h>
#include <geGL/StaticCalls.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <tiny_obj_loader.h>
#include "gl_basics.h"

struct ObjFileInfo {
    std::string name;
    int instanceCount;
};

void loadObjectsFromFiles(std::vector<ObjFileInfo>objectToLoad, std::vector<Material>& mats, std::vector<Object>& objects);