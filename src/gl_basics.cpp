#include "gl_basics.h"

using namespace ge::gl;

#ifndef RESOURCE_DIR
#define RESOURCE_DIR "."
#endif

#ifndef SHADERS_DIR
#define SHADERS_DIR "."
#endif

GLuint loadTexture(const std::string& path) 
{
    int w, h, ch;
    unsigned char* data = stbi_load(path.c_str(), &w, &h, &ch, 0);
    if (!data) { std::cerr << "Texture load failed: " << path << std::endl; return 0; }

    GLenum format = (ch == 4) ? GL_RGBA : GL_RGB;
    GLuint tex; 
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    
    glPixelStorei(GL_PACK_ROW_LENGTH, w);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, w);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, h);
    
    glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
    return tex;
}


GLuint loadShader(GLenum type, const char* path) 
{
    FILE* f = fopen(path, "r");
    fseek(f, 0, SEEK_END); long len = ftell(f); rewind(f);
    std::string src(len, '\0'); fread(&src[0], 1, len, f); fclose(f);

    GLuint shader = glCreateShader(type);
    const char* s = src.c_str();
    glShaderSource(shader, 1, &s, NULL);
    glCompileShader(shader);

    // Handle compilation errors
    GLint isCompiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
    if(GL_FALSE == isCompiled)
    {
        GLint maxLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

        // The maxLength includes the NULL character
        std::vector<GLchar> errorLog(maxLength);
        glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);

        // Provide the infolog
        for (auto i: errorLog)
            std::cout << i;
        // Exit with failure.
        glDeleteShader(shader); // Don't leak the shader.
        return GL_FALSE;
    }
    return shader;
}

GLuint createProgram(const char* vsPath, const char* fsPath) 
{
    GLuint vs = loadShader(GL_VERTEX_SHADER, vsPath);
    GLuint fs = loadShader(GL_FRAGMENT_SHADER, fsPath);
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);
    glDeleteShader(vs);
    glDeleteShader(fs);
    return prog;
}

void renderObjects(GLuint program, std::vector<Material>& mats, std::vector<Object>& objects)
{
    glUseProgram(program);
    for (const auto& object : objects) 
    {
        for (const auto& mesh : object.meshes) 
        {
            const Material& mat = (mesh.materialID >= 0) ? mats[mesh.materialID] : mats[0];
            
            if (mat.diffuseTex) {
                glBindTextureUnit(0, mat.diffuseTex);
                glUniform1i(glGetUniformLocation(program, "useTexture"), 1);
            } else {
                glUniform1i(glGetUniformLocation(program, "useTexture"), 0);
                glUniform3fv(glGetUniformLocation(program, "diffuseColor"), 1, (float*)&(mat.diffuseColor));
            }
            glBindTextureUnit(1, depthMapTexture); // bind shadow map
            
            glBindVertexArray(mesh.vao);

            glDrawElementsInstanced(
                GL_TRIANGLES,
                mesh.vertexCount,
                GL_UNSIGNED_INT,
                nullptr,
                object.maxInstanceCount
            );
        }
    }
}


bool aabbOutsidePlane(const AABB& box, const Plane& p)
{
    glm::vec3 positive = box.min;

    if (p.n.x >= 0) positive.x = box.max.x;
    if (p.n.y >= 0) positive.y = box.max.y;
    if (p.n.z >= 0) positive.z = box.max.z;

    return glm::dot(p.n, positive) + p.d < 0;
}

bool aabbInFrustum(const AABB& box, const std::array<Plane, 6>& frustum)
{
    for (const auto& plane : frustum) {
        if (aabbOutsidePlane(box, plane))
            return false;
    }
    return true;
}

void extractFrustumPlanes(const glm::mat4& vp, std::array<Plane, 6>& planes)
{
    // left
    planes[0].n.x = vp[0][3] + vp[0][0];
    planes[0].n.y = vp[1][3] + vp[1][0];
    planes[0].n.z = vp[2][3] + vp[2][0];
    planes[0].d   = vp[3][3] + vp[3][0];

    // right
    planes[1].n.x = vp[0][3] - vp[0][0];
    planes[1].n.y = vp[1][3] - vp[1][0];
    planes[1].n.z = vp[2][3] - vp[2][0];
    planes[1].d   = vp[3][3] - vp[3][0];

    // bottom
    planes[2].n.x = vp[0][3] + vp[0][1];
    planes[2].n.y = vp[1][3] + vp[1][1];
    planes[2].n.z = vp[2][3] + vp[2][1];
    planes[2].d   = vp[3][3] + vp[3][1];

    // top
    planes[3].n.x = vp[0][3] - vp[0][1];
    planes[3].n.y = vp[1][3] - vp[1][1];
    planes[3].n.z = vp[2][3] - vp[2][1];
    planes[3].d   = vp[3][3] - vp[3][1];

    // near
    planes[4].n.x = vp[0][3] + vp[0][2];
    planes[4].n.y = vp[1][3] + vp[1][2];
    planes[4].n.z = vp[2][3] + vp[2][2];
    planes[4].d   = vp[3][3] + vp[3][2];

    // far
    planes[5].n.x = vp[0][3] - vp[0][2];
    planes[5].n.y = vp[1][3] - vp[1][2];
    planes[5].n.z = vp[2][3] - vp[2][2];
    planes[5].d   = vp[3][3] - vp[3][2];

    // normalize
    for (auto& p : planes) {
        float len = glm::length(p.n);
        p.n /= len;
        p.d /= len;
    }
}