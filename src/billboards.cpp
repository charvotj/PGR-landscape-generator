#include "billboards.h"

using namespace ge::gl;

#ifndef RESOURCE_DIR
#define RESOURCE_DIR "."
#endif

#ifndef SHADERS_DIR
#define SHADERS_DIR "."
#endif

void updateViewProjBillboards(GLuint program, Object& objectToPrint, float viewAngle)
{
    float height = objectToPrint.boundingBox.max.y - objectToPrint.boundingBox.min.y;
    auto view = glm::mat4(1.f);
    auto proj = glm::ortho(-height/2.f, height/2.f, 0.0f, height, -10.f, 10.f);

    auto viewProjL = glGetUniformLocation(program, "viewProj");
    view = glm::rotate(view, viewAngle, glm::vec3(0.f, 1.f, 0.f));

    auto viewProj = proj * view;

    glProgramUniformMatrix4fv(program, viewProjL, 1, GL_FALSE, (float*)&viewProj);
}

GLuint renderTreeBillboardTexture(GLuint program, std::vector<Material>& mats, Object& treeObj, float viewAngle)
{
    int textureSize = 1024;
    GLuint fbo, treeTexture;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Create empty texture
    glGenTextures(1, &treeTexture);
    glBindTexture(GL_TEXTURE_2D, treeTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureSize, textureSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    // Filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // Avoid texture repeat at corners
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Připojení textury k FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, treeTexture, 0);

    // Nutný Depth Buffer pro 3D model, aby se vykreslil správně
    GLuint rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, textureSize, textureSize);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "FBO Error!" << std::endl;

    glViewport(0, 0, textureSize, textureSize);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glUseProgram(program);
    updateViewProjBillboards(program, treeObj, viewAngle);
    
    for (const auto& mesh : treeObj.meshes) 
    {
        const Material& mat = (mesh.materialID >= 0) ? mats[mesh.materialID] : mats[0];
        
        if (mat.diffuseTex) {
            glBindTextureUnit(0, mat.diffuseTex);
            glUniform1i(glGetUniformLocation(program, "useTexture"), 1);
        } else {
            glUniform1i(glGetUniformLocation(program, "useTexture"), 0);
            glUniform3fv(glGetUniformLocation(program, "diffuseColor"), 1, (float*)&(mat.diffuseColor));
        }
        
        glBindVertexArray(mesh.vao);
        glDrawElements(
            GL_TRIANGLES,
            mesh.vertexCount,
            GL_UNSIGNED_INT,
            nullptr
        );
    }
    // Generate Mipmaps:
    glBindTexture(GL_TEXTURE_2D, treeTexture);
    glGenerateMipmap(GL_TEXTURE_2D);
    return treeTexture;
}

Mesh createQuad(float angle, float height, uint8_t matId, GLuint instanceVBO)
{
    Mesh mesh;
    float width = height;
    float halfWidth = width / 2.0f;

    // Pomocná funkce pro rotaci bodu kolem osy Y
    auto rotateY = [angle](glm::vec3 p) {
        return glm::vec3(
            p.x * cos(angle) + p.z * sin(angle),
            p.y,
            -p.x * sin(angle) + p.z * cos(angle)
        );
    };

    // Quad vertices
    std::vector<Vertex> vertices(4);

    vertices[0].pos = rotateY(glm::vec3(-halfWidth, 0.0f, 0.0f));
    vertices[0].uv  = glm::vec2(0.0f, 0.0f);

    vertices[1].pos = rotateY(glm::vec3(halfWidth, 0.0f, 0.0f));
    vertices[1].uv  = glm::vec2(1.0f, 0.0f);

    vertices[2].pos = rotateY(glm::vec3(halfWidth, height, 0.0f));
    vertices[2].uv  = glm::vec2(1.0f, 1.0f);

    vertices[3].pos = rotateY(glm::vec3(-halfWidth, height, 0.0f));
    vertices[3].uv  = glm::vec2(0.0f, 1.0f);

    // Quad indices
    std::vector<uint32_t> indices = {
        0, 1, 2, // První trojúhelník
        2, 3, 0  // Druhý trojúhelník
    };

    // Bounding Box
    mesh.boundingBox.min = glm::vec3(-halfWidth, 0.0f, -halfWidth);
    mesh.boundingBox.max = glm::vec3(halfWidth, height, halfWidth);

    // VAO
    glCreateVertexArrays(1, &mesh.vao);

    // VBO
    glCreateBuffers(1, &mesh.vbo);
    glNamedBufferData(mesh.vbo, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
    glVertexArrayVertexBuffer(mesh.vao, 0, mesh.vbo, 0, sizeof(Vertex));

    // EBO
    glCreateBuffers(1, &mesh.ebo);
    glNamedBufferData(mesh.ebo, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);
    glVertexArrayElementBuffer(mesh.vao, mesh.ebo);

    // Attribs
    glEnableVertexArrayAttrib(mesh.vao, 0); // pos
    glVertexArrayAttribFormat(mesh.vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, pos));
    glVertexArrayAttribBinding(mesh.vao, 0, 0);

    glEnableVertexArrayAttrib(mesh.vao, 1); // uv
    glVertexArrayAttribFormat(mesh.vao, 1, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, uv));
    glVertexArrayAttribBinding(mesh.vao, 1, 0);

    /* ---------- shared instance VBO ---------- */
    glVertexArrayVertexBuffer(mesh.vao, 1, instanceVBO, 0, sizeof(InstanceData));
    /* mat4 = 4× vec4 (atributs 2–5) */
    for (int i = 0; i < 4; ++i) {
        glEnableVertexArrayAttrib(mesh.vao, 2 + i);
        glVertexArrayAttribFormat(
            mesh.vao,
            2 + i,
            4,
            GL_FLOAT,
            GL_FALSE,
            offsetof(InstanceData, model) + sizeof(glm::vec4) * i
        );
        glVertexArrayAttribBinding(mesh.vao, 2 + i, 1);
    }

    /* divisor = per-instance */
    glVertexArrayBindingDivisor(mesh.vao, 1, 1);

    mesh.vertexCount = indices.size();
    mesh.materialID = matId;

    return mesh;
}

void addBillboardTrees(std::vector<Material>& mats, std::vector<Object>& treeObjects, std::vector<Object>& treeBillboards)
{
    // Input 3D trees
    for (auto& treeObj : treeObjects)
    {
        // Output 2D tree
        Object crossQuadTree;
        crossQuadTree.name = "billboard_" + treeObj.name;
        int instanceCount = 100;
        
        /* INSTANCING */      
        /* ---------- shared instance VBO ---------- */
        crossQuadTree.maxInstanceCount = treeObj.maxInstanceCount;
        glCreateBuffers(1, &crossQuadTree.instanceVBO);
        glNamedBufferData(crossQuadTree.instanceVBO, crossQuadTree.maxInstanceCount * sizeof(InstanceData), NULL, GL_DYNAMIC_DRAW);   
        
        float angleFront = 0.f;
        float angleSide = glm::pi<float>() / 2.f;
        float height = treeObj.boundingBox.max.y - treeObj.boundingBox.min.y;
        
        Mesh meshFront, meshSide;
        Material matFront, matSide;
        
        auto program = createProgram(SHADERS_DIR "billboard.vs", SHADERS_DIR "billboard.fs");
        matFront.diffuseTex = renderTreeBillboardTexture(program, mats, treeObj, angleFront);
        mats.push_back(matFront);
        meshFront = createQuad(angleFront, height, mats.size() - 1, crossQuadTree.instanceVBO);
        crossQuadTree.meshes.push_back(meshFront);
    
        matSide.diffuseTex = renderTreeBillboardTexture(program, mats, treeObj, angleSide);
        mats.push_back(matSide);
        meshSide = createQuad(angleSide, height, mats.size() - 1, crossQuadTree.instanceVBO);
        crossQuadTree.meshes.push_back(meshSide);
    
        treeBillboards.push_back(crossQuadTree);
    }
}