#pragma once

#include <iostream>
#include <fstream>
#include <geGL/geGL.h>
#include <geGL/StaticCalls.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "gl_basics.h"

struct Skybox {
    GLuint vao;
    GLuint vbo;
    size_t vertexCount;
    GLuint cubemapTexture;
    GLuint program;
};

Skybox createSkyBox(const std::string& skyboxName);
void renderSkybox(Skybox& skybox);