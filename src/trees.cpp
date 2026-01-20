#include "trees.h"

using namespace ge::gl;

#ifndef RESOURCE_DIR
#define RESOURCE_DIR "."
#endif

#ifndef SHADERS_DIR
#define SHADERS_DIR "."
#endif

struct DrawElementsIndirectCommand {
    GLuint count;           //  (mesh.vertexCount)
    GLuint instanceCount;   // updated in Compute Shader!
    GLuint firstIndex;
    GLint  baseVertex;
    GLuint baseInstance;
};


GLfloat lodThreshold = 1.0f;
bool lodThresholdChanged = false;
bool frustumCullingEnabled = false;

void setLodThreshold(GLfloat threshold)
{
    lodThreshold = threshold;
    lodThresholdChanged = true;
}

void setFrustumCulling(bool state)
{
    frustumCullingEnabled = state;
    lodThresholdChanged = true;
}


std::vector<glm::vec3> loadTreePositions(const std::string& filename) 
{
    std::ifstream file((RESOURCE_DIR "bin/" + filename).c_str(), std::ios::binary);
    if (!file) throw std::runtime_error("Nelze otevřít soubor: " + filename);

    // file size
    file.seekg(0, std::ios::end);
    size_t file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    // load data from file 
    std::vector<glm::vec3> buffer(file_size / sizeof(glm::vec3));
    file.read(reinterpret_cast<char*>(buffer.data()), file_size);

    return buffer;
}

void loadTrees(std::vector<Material>& mats, std::vector<Tree>& trees)
{
    // Load models
    std::vector<ObjFileInfo> objectToLoad = {
        {"douglas_fir.obj", 2000},
        {"picea.obj", 2000},
    };

    std::vector<glm::vec3> treePositions = loadTreePositions(std::string(TERRAIN_TILE_NUMBER) + ".trees.bin");
    int treePosOffset = 0;

    std::vector<Object> treeObjects;
    loadObjectsFromFiles(objectToLoad, mats, treeObjects);
    
    std::vector<Object> treeBillboards;
    addBillboardTrees(mats, treeObjects, treeBillboards);

    if(treeBillboards.size() != treeObjects.size())
    {
        std::cerr << "Error loading trees";
        return;
    }
    srand(44); // seed
    for(uint8_t i = 0u; i < treeObjects.size(); i++)
    {
        Tree newTreeKind;

        newTreeKind.tree3D = treeObjects[i];
        newTreeKind.treeBillboard = treeBillboards[i];
        newTreeKind.name = newTreeKind.tree3D.name;

        // Calculate instance model matrices
        for (int j = 0; j < newTreeKind.tree3D.maxInstanceCount; ++j) 
        {
            float randScale = 0.8f + ((rand() % 41)) / 100.0f;
            float scaleFactor = 1.f / 2000.0f;
            InstanceData iData;
            iData.model = glm::translate(glm::mat4(1.0f), treePositions[treePosOffset++]);
            iData.model = glm::scale(iData.model, glm::vec3(scaleFactor * randScale));
            newTreeKind.instances.push_back(iData);

            if((treePosOffset) == treePositions.size())
            {
                std::cerr << "Maximum tree positions used";
                break;
            }
        }
        // Create real instanceVBO with data - used as a source in compute shader
        glCreateBuffers(1, &newTreeKind.instanceVBO);
        glNamedBufferData(newTreeKind.instanceVBO, newTreeKind.instances.size() * sizeof(InstanceData), newTreeKind.instances.data(), GL_DYNAMIC_DRAW);   

        // Vytvoření Indirect Bufferu (pro 2 příkazy: LOD0 a LOD1)
        DrawElementsIndirectCommand cmds[2] = { 0 };
        glGenBuffers(1, &newTreeKind.indirectBuffer);
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, newTreeKind.indirectBuffer);
        glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(cmds), cmds, GL_DYNAMIC_DRAW);

        trees.push_back(newTreeKind);
    }
}

GLuint computeProgram;
void treesInit()
{
    GLuint cs = loadShader(GL_COMPUTE_SHADER, SHADERS_DIR "lod.comp");
    computeProgram = glCreateProgram();
    glAttachShader(computeProgram, cs);
    glLinkProgram(computeProgram);
    glDeleteShader(cs);
}

glm::vec3 cameraPosOld = glm::vec3(0.f);
void renderTrees(GLuint renderProgram, std::vector<Material>& mats, std::vector<Tree>& trees)
{
    bool computeNewLodMap = getCameraChangeStateForLod();
    
    for(const auto& tree : trees)
    {
        if(computeNewLodMap)
        {
            // reset instance count before calling compute shader
            glBindBuffer(GL_COPY_WRITE_BUFFER, tree.indirectBuffer);
            GLuint zero = 0u;
            glBufferSubData(GL_COPY_WRITE_BUFFER, offsetof(DrawElementsIndirectCommand, instanceCount), sizeof(GLuint), &zero); // LOD0
            glBufferSubData(GL_COPY_WRITE_BUFFER, sizeof(DrawElementsIndirectCommand) + offsetof(DrawElementsIndirectCommand, instanceCount), sizeof(GLuint), &zero); // LOD1
            
            
            // run Compute Shader
            glUseProgram(computeProgram);
            auto cameraPos = getCameraPosition();
            glUniform3fv(glGetUniformLocation(computeProgram, "cameraPos"), 1, &cameraPos[0]);
            glUniform1f(glGetUniformLocation(computeProgram, "lodThreshold"), (GLfloat)lodThreshold);
            glUniform1ui(glGetUniformLocation(computeProgram, "totalCount"), (GLuint)tree.instances.size());
            std::array<Plane, 6> frustumPlanes;
            extractFrustumPlanes(getCameraViewProj(), frustumPlanes);
            glUniform4fv(glGetUniformLocation(computeProgram, "frustumPlanes"), 6, (float*)frustumPlanes.data());
            glUniform1i(glGetUniformLocation(computeProgram, "useFrustumCulling"), frustumCullingEnabled);
    
            // buffer bindings
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, tree.instanceVBO);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, tree.tree3D.instanceVBO);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, tree.treeBillboard.instanceVBO);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, tree.indirectBuffer); // Ten s těmi příkazy
    
            // run it!
            glDispatchCompute((tree.instances.size() + 63) / 64, 1, 1);
        
            // wait until finished
            glMemoryBarrier(GL_COMMAND_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
        }

        glUseProgram(renderProgram);

        for (const auto& mesh : tree.tree3D.meshes) 
        {
            // Update vertex count in indirect buffer
            glBindBuffer(GL_DRAW_INDIRECT_BUFFER, tree.indirectBuffer);
            glBufferSubData(GL_DRAW_INDIRECT_BUFFER, offsetof(DrawElementsIndirectCommand, count), sizeof(GLuint), &mesh.vertexCount); // LOD0
            glMemoryBarrier(GL_COMMAND_BARRIER_BIT);

            const Material& mat = (mesh.materialID >= 0) ? mats[mesh.materialID] : mats[0];
            
            if (mat.diffuseTex) {
                glBindTextureUnit(0, mat.diffuseTex);
                glUniform1i(glGetUniformLocation(renderProgram, "useTexture"), 1);
            } else {
                glUniform1i(glGetUniformLocation(renderProgram, "useTexture"), 0);
                glUniform3fv(glGetUniformLocation(renderProgram, "diffuseColor"), 1, (float*)&(mat.diffuseColor));
            }
            glBindTextureUnit(1, depthMapTexture); // bind shadow map
            
            glBindVertexArray(mesh.vao);

            // glMemoryBarrier(GL_ALL_BARRIER_BITS);
            glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (void*)0);
        }

        for (const auto& mesh : tree.treeBillboard.meshes) 
        {
            // Update vertex count in indirect buffer
            glBindBuffer(GL_DRAW_INDIRECT_BUFFER, tree.indirectBuffer);
            glBufferSubData(GL_DRAW_INDIRECT_BUFFER, sizeof(DrawElementsIndirectCommand) + offsetof(DrawElementsIndirectCommand, count), sizeof(GLuint), &mesh.vertexCount); // LOD1
            glMemoryBarrier(GL_COMMAND_BARRIER_BIT);

            const Material& mat = (mesh.materialID >= 0) ? mats[mesh.materialID] : mats[0];
            
            if (mat.diffuseTex) {
                glBindTextureUnit(0, mat.diffuseTex);
                glUniform1i(glGetUniformLocation(renderProgram, "useTexture"), 1);
            } else {
                glUniform1i(glGetUniformLocation(renderProgram, "useTexture"), 0);
                glUniform3fv(glGetUniformLocation(renderProgram, "diffuseColor"), 1, (float*)&(mat.diffuseColor));
            }
            
            glBindVertexArray(mesh.vao);

            // glMemoryBarrier(GL_ALL_BARRIER_BITS);
            glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (void*)sizeof(DrawElementsIndirectCommand));
        }
    }
}