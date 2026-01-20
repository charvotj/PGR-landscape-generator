#include "skybox.h"

using namespace ge::gl;

#ifndef RESOURCE_DIR
#define RESOURCE_DIR "."
#endif

#ifndef SHADERS_DIR
#define SHADERS_DIR "."
#endif

GLuint loadCubemapTexture(const std::string& skyboxName)
{
    std::string skyboxPath = std::string(RESOURCE_DIR) + "skybox/cubemaps/" + skyboxName + "/";
    std::vector<std::string> faces = {
        "right.jpg",
        "left.jpg",
        "top.jpg",
        "bottom.jpg",
        "front.jpg",
        "back.jpg"
    };

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int w, h, channels;

    for (GLuint i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load((skyboxPath + faces[i]).c_str(), &w, &h, &channels, 0);

        if (!data) {
            std::cerr << "Failed to load cubemap face: " << faces[i] << std::endl;
            continue;
        }

        GLenum format = (channels == 3) ? GL_RGB : GL_RGBA;

        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                     0, format, w, h, 0, format, GL_UNSIGNED_BYTE, data);

        stbi_image_free(data);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}


Skybox createSkyBox(const std::string& skyboxName)
{
    Skybox newSkybox;

    float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f,  1.0f
    };

    glGenVertexArrays(1, &(newSkybox.vao));
    glGenBuffers(1, &(newSkybox.vbo));

    glBindVertexArray(newSkybox.vao);
    glBindBuffer(GL_ARRAY_BUFFER, newSkybox.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    newSkybox.vertexCount = sizeof(skyboxVertices) / (3 * sizeof(float));
    newSkybox.cubemapTexture = loadCubemapTexture(skyboxName);
    newSkybox.program = createProgram(SHADERS_DIR "skyboxVertex.vs", SHADERS_DIR "skyboxFragment.fs");

    return newSkybox;
}

void renderSkybox(Skybox& skybox)
{
    // disable depth write - skybox should be on background
    glDepthFunc(GL_LEQUAL); // not GL_LESS !
    glDepthMask(GL_FALSE);

    glUseProgram(skybox.program);

    glBindVertexArray(skybox.vao);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skybox.cubemapTexture);

    glDrawArrays(GL_TRIANGLES, 0, skybox.vertexCount);

    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);
}