#pragma once

#include <iostream>
#include <fstream>
#include <geGL/geGL.h>
#include <geGL/StaticCalls.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "gl_basics.h"
#include "global_defines.h"

Mesh createTerrainMesh(uint8_t matID);
Material createTerrainMaterial();

void loadTerrain(std::vector<Material>& mats, std::vector<Object>& objects);
void renderTerrain(GLuint program, std::vector<Material>& mats, std::vector<Object>& terrainTiles);