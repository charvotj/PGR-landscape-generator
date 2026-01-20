#include "obj_load.h"

using namespace ge::gl;

#ifndef RESOURCE_DIR
#define RESOURCE_DIR "."
#endif

#ifndef SHADERS_DIR
#define SHADERS_DIR "."
#endif


void loadObjectsFromFiles(std::vector<ObjFileInfo>objectToLoad, std::vector<Material>& mats, std::vector<Object>& objects)
{
    for (auto objModel : objectToLoad)
    {
        Object loadedObj;
        loadedObj.name = objModel.name;
        loadedObj.boundingBox.min = glm::vec3(0.f);
        loadedObj.boundingBox.max = glm::vec3(0.f);

        auto matIdOffset = mats.size(); // matterials from previous objects
        // --- Loading by TinyObjLoader ---
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;
    
        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, (RESOURCE_DIR + objModel.name).c_str(), RESOURCE_DIR)) {
            std::cerr << "Error loading OBJ: " << err << std::endl;
            return;
        }
    
        // Load material
        for (auto& m : materials) {
            Material mat;
            mat.diffuseColor = glm::vec3(m.diffuse[0], m.diffuse[1], m.diffuse[2]);
            if (!m.diffuse_texname.empty())
                mat.diffuseTex = loadTexture(std::string(RESOURCE_DIR) + m.diffuse_texname);
            mats.push_back(mat);
        }

        /* INSTANCING */
        loadedObj.maxInstanceCount = objModel.instanceCount;
        /* ---------- shared instance VBO ---------- */
        glCreateBuffers(1, &loadedObj.instanceVBO);
        glNamedBufferData(loadedObj.instanceVBO, loadedObj.maxInstanceCount * sizeof(InstanceData), NULL, GL_DYNAMIC_DRAW);   
    
        // Load meshes
        for (const auto& shape : shapes) {
            Mesh mesh;
            std::unordered_map<Vertex, uint32_t> uniqueVertices;
            std::vector<Vertex> vertices;
            std::vector<uint32_t> indices;

            // initialize bounding box
            mesh.boundingBox.min = glm::vec3(0.f);
            mesh.boundingBox.max = glm::vec3(0.f);

            for (const auto& idx : shape.mesh.indices) 
            {
                Vertex vertex{};
                vertex.pos = {
                    attrib.vertices[3 * idx.vertex_index + 0],
                    attrib.vertices[3 * idx.vertex_index + 1],
                    attrib.vertices[3 * idx.vertex_index + 2]
                };
                vertex.uv = {0.0f, 0.0f};
                if (idx.texcoord_index >= 0) 
                {
                    vertex.uv = {
                        attrib.texcoords[2 * idx.texcoord_index + 0],
                        1.0f - attrib.texcoords[2 * idx.texcoord_index + 1]
                    };
                }

                if (uniqueVertices.count(vertex) == 0) 
                {
                    uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                    
                    // update AABB bounding box - only for new unique vertex
                    for(uint8_t i = 0; i<3;i++)
                    {
                        if(vertex.pos[i] > mesh.boundingBox.max[i])
                            mesh.boundingBox.max[i] = vertex.pos[i];
                        else if (vertex.pos[i] < mesh.boundingBox.min[i])
                            mesh.boundingBox.min[i] = vertex.pos[i];
                    }
                }
                indices.push_back(uniqueVertices[vertex]);
            }
            mesh.materialID = shape.mesh.material_ids.empty() ? -1 : matIdOffset + shape.mesh.material_ids[0];
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

            /* ---------- shared instance VBO ---------- */
            glVertexArrayVertexBuffer(mesh.vao, 1, loadedObj.instanceVBO, 0, sizeof(InstanceData));
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
            
            loadedObj.meshes.push_back(mesh);

            // update object AABB bounding box - from new mesh
            for(uint8_t i = 0; i<3;i++)
            {
                if(mesh.boundingBox.max[i] > loadedObj.boundingBox.max[i])
                    loadedObj.boundingBox.max[i] = mesh.boundingBox.max[i];
                else if (mesh.boundingBox.min[i] < loadedObj.boundingBox.min[i])
                    loadedObj.boundingBox.min[i] = mesh.boundingBox.min[i];
            }
        }
        objects.push_back(loadedObj);  
    }
}