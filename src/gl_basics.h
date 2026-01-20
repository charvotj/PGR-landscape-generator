#pragma once

#include <geGL/geGL.h>
#include <geGL/StaticCalls.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>
#include "shadows.h"

struct Vertex { 
    glm::vec3 pos; 
    glm::vec2 uv; 

    bool operator==(const Vertex& other) const {
        return pos == other.pos && uv == other.uv;
    }
};

struct AABB {
    glm::vec3 min;
    glm::vec3 max;
};

struct Plane {
    glm::vec3 n;
    float d;
};

struct InstanceData {
    glm::mat4 model;   // transformace
};

struct Material {
    glm::vec3 diffuseColor = {1.0f, 1.0f, 1.0f};
    glm::vec3 specularColor = {0.0f, 0.0f, 0.0f};
    float shininess = 32.0f;
    GLuint diffuseTex = 0;
    GLuint specularTex = 0;
};

struct Mesh {
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    size_t vertexCount;
    int materialID;
    AABB boundingBox;
};

struct Object {
    std::string name;
    GLuint instanceVBO;
    std::vector<Mesh> meshes;
    AABB boundingBox;
    int maxInstanceCount;
};

namespace std {
    template<> struct hash<Vertex> {
        size_t operator()(const Vertex& v) const {
            return ((hash<float>()(v.pos.x) ^ (hash<float>()(v.pos.y) << 1)) >> 1) ^
                   (hash<float>()(v.pos.z) << 1) ^ (hash<float>()(v.uv.s) << 2);
        }
    };
}

GLuint loadTexture(const std::string& path);
GLuint loadShader(GLenum type, const char* path);
GLuint createProgram(const char* vsPath, const char* fsPath);

void renderObjects(GLuint program, std::vector<Material>& mats, std::vector<Object>& objects);
void extractFrustumPlanes(const glm::mat4& vp, std::array<Plane, 6>& planes);