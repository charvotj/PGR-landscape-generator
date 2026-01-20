#include "terrain.h"

using namespace ge::gl;

#ifndef RESOURCE_DIR
#define RESOURCE_DIR "."
#endif

#ifndef SHADERS_DIR
#define SHADERS_DIR "."
#endif


std::vector<Vertex> loadPointCloud(const std::string& filename) 
{
    std::ifstream file((RESOURCE_DIR "bin/" + filename).c_str(), std::ios::binary);
    if (!file) throw std::runtime_error("Nelze otevřít soubor: " + filename);

    // file size
    file.seekg(0, std::ios::end);
    size_t file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    // load data from file 
    std::vector<float> buffer(file_size / sizeof(float));
    file.read(reinterpret_cast<char*>(buffer.data()), file_size);

    std::vector<Vertex> vertexes;
    vertexes.reserve(buffer.size() / 3);
    for (size_t i = 0; i < buffer.size(); i += 3) {
        Vertex v = {
            {buffer[i], buffer[i + 1], buffer[i + 2]},
            {1.0 - buffer[i], 1.0 - (buffer[i + 2])}
        };
        vertexes.emplace_back(v);
    }

    return vertexes;
}

std::vector<uint32_t> loadPointCloudIndexes(const std::string& filename)
{
    std::ifstream file((RESOURCE_DIR "bin/" + filename).c_str(), std::ios::binary);
    if (!file) throw std::runtime_error("Nelze otevřít soubor: " + filename);

    // file size
    file.seekg(0, std::ios::end);
    size_t file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    // load data from file 
    std::vector<uint32_t> buffer(file_size / sizeof(uint32_t));
    file.read(reinterpret_cast<char*>(buffer.data()), file_size);

    return buffer;
}

void loadTerrainMesh(Object& terrain, uint8_t matID)
{
    Mesh mesh;
    std::vector<Vertex> vertices = loadPointCloud(terrain.name + ".bin");
    std::vector<uint32_t> indices = loadPointCloudIndexes(terrain.name + ".idx.bin");

    mesh.materialID = matID;
    mesh.vertexCount = indices.size();


    /* ---------- VAO ---------- */
    glCreateVertexArrays(1, &mesh.vao);

    /* ---------- VBO ---------- */
    glCreateBuffers(1, &mesh.vbo);
    glNamedBufferData(mesh.vbo, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glVertexArrayVertexBuffer(mesh.vao, 0, mesh.vbo, 0, sizeof(Vertex));
    
    /* pos */
    glEnableVertexArrayAttrib(mesh.vao, 0);
    glVertexArrayAttribFormat(mesh.vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, pos));
    glVertexArrayAttribBinding(mesh.vao, 0, 0);
    
    /* uv */
    glEnableVertexArrayAttrib(mesh.vao, 1);
    glVertexArrayAttribFormat(mesh.vao, 1, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, uv));
    glVertexArrayAttribBinding(mesh.vao, 1, 0);
    

    /* ---------- index buffer (EBO) ---------- */
    glCreateBuffers(1, &mesh.ebo);
    glNamedBufferData(mesh.ebo, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);

    glVertexArrayElementBuffer(mesh.vao, mesh.ebo);
    

    /* INSTANCING - only 1 instance*/
    terrain.maxInstanceCount = 1;
    InstanceData iData;
    iData.model = glm::mat4(1.0f);
    /* ---------- instance VBO ---------- */
    glCreateBuffers(1, &terrain.instanceVBO);
    glNamedBufferData(terrain.instanceVBO, 1 * sizeof(InstanceData), &iData, GL_DYNAMIC_DRAW);
    glVertexArrayVertexBuffer(mesh.vao, 1, terrain.instanceVBO, 0, sizeof(InstanceData));
    /* mat4 = 4× vec4 (atributy 2–5) */
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

    /* Calculate bounding boxes*/
    for(const auto& vertex : vertices)
    {
        for(uint8_t i = 0; i<3;i++)
        {
            if(vertex.pos[i] > mesh.boundingBox.max[i])
                mesh.boundingBox.max[i] = vertex.pos[i];
            else if (vertex.pos[i] < mesh.boundingBox.min[i])
                mesh.boundingBox.min[i] = vertex.pos[i];
        }
    }

    terrain.meshes.push_back(mesh);
    terrain.boundingBox = mesh.boundingBox;
}

Material createTerrainMaterial(std::string terrainName)
{
    Material mat;
    mat.diffuseColor = glm::vec3(0.1f,0.8f,0.1f);

    mat.diffuseTex = loadTexture(std::string(RESOURCE_DIR) + terrainName + ".jpg");
    return mat;
}


void loadTerrain(std::vector<Material>& mats, std::vector<Object>& objects)
{
    Object terrain;
    terrain.name = TERRAIN_TILE_NUMBER;

    auto terrainMat = createTerrainMaterial(terrain.name);
    mats.push_back(terrainMat);
    
    loadTerrainMesh(terrain, mats.size() - 1);
    objects.push_back(terrain);
}

void renderTerrain(GLuint program, std::vector<Material>& mats, std::vector<Object>& terrainTiles)
{
    renderObjects(program, mats, terrainTiles);
}

