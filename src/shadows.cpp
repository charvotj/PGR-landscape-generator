#include "shadows.h"

using namespace ge::gl;

#ifndef RESOURCE_DIR
#define RESOURCE_DIR "."
#endif

#ifndef SHADERS_DIR
#define SHADERS_DIR "."
#endif

#define SHADOW_WIDTH 8192
#define SHADOW_HEIGHT 8192

GLuint depthMapFBO;
GLuint depthMapProgram;
GLuint depthMapTexture;


float near_plane = 0.1f, far_plane = 1.1f;
glm::mat4 lightProjection = glm::ortho(-0.7f, 0.7f, -0.15f, 0.45f, near_plane, far_plane); 

glm::mat4 lightView = glm::lookAt(glm::vec3(1.1f, 0.5f, 0.5f), 
                                  glm::vec3( 0.5f, 0.2f,  0.5f), 
                                  glm::vec3( 0.0f, 1.0f,  0.0f)); 

glm::mat4 lightViewProj = lightProjection * lightView;


void shadowMapInit()
{
    depthMapProgram = createProgram(SHADERS_DIR "shadows.vs", SHADERS_DIR "shadows.fs");


    glGenFramebuffers(1, &depthMapFBO);



    glGenTextures(1, &depthMapTexture);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);



    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);  
}

void programLightViewProj(GLuint program)
{
    auto viewProjL = glGetUniformLocation(program, "lightViewProj");
    glProgramUniformMatrix4fv(program, viewProjL, 1, GL_FALSE, (float*)&lightViewProj);
}

// After calling this function you can render scene to shadow map
void prepareDepthMapRender()
{
    glUseProgram(depthMapProgram);
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    programLightViewProj(depthMapProgram);
}