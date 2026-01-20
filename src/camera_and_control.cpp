#include "camera_and_control.h"

using namespace ge::gl;

#ifndef RESOURCE_DIR
#define RESOURCE_DIR "."
#endif

#ifndef SHADERS_DIR
#define SHADERS_DIR "."
#endif

auto cameraPos = glm::vec3(-0.176850826f, -0.405259955f, -0.0869411603f);
auto cameraAngleZ = 3.43779588f;
auto cameraAngleUpDown = 0.0272068847f;
auto cameraSpeed = 1.f;

std::map<int, bool> keys;

auto view = glm::mat4(1.f);
auto proj = glm::perspective(glm::pi<float>() / 2.f, 1.77f, 0.001f, 20.f);
auto viewProj = glm::mat4(1.f);
bool viewProjChanged = false;

bool cameraChangeStateForLod = true;

glm::vec3 getCameraPosition()
{
    return -cameraPos;
}

glm::mat4 getCameraViewProj()
{
    return viewProj;
}

bool getCameraChangeStateForLod()
{
    auto res = cameraChangeStateForLod;
    cameraChangeStateForLod = false;

    return res;
}

bool getCameraViewProjChange()
{
    auto res = viewProjChanged;
    viewProjChanged = false;

    return res;
}

void cameraUpdateAngle(float xrel, float yrel)
{
    cameraAngleUpDown += yrel * 0.01f;
    cameraAngleZ += xrel * 0.01f;
}

void cameraUpdatePosition()
{   
    viewProjChanged = true;

    glm::mat4 viewPrev = glm::mat4(1.f);
    viewPrev = glm::rotate(viewPrev, cameraAngleZ, glm::vec3(0.f, 1.f, 0.f));
    
    cameraPos += 0.001f * cameraSpeed * glm::vec3(glm::transpose(viewPrev)[2]) * (float)((keys[SDLK_W]) - (keys[SDLK_S]));
    cameraPos += 0.001f * cameraSpeed * glm::vec3(glm::transpose(viewPrev)[1]) * (float)((keys[SDLK_LSHIFT]) - (keys[SDLK_SPACE]));
    cameraPos += 0.001f * cameraSpeed * glm::vec3(glm::transpose(viewPrev)[0]) * (float)((keys[SDLK_A]) - (keys[SDLK_D]));
    
    if(cameraPos[1] > 0.0f) cameraPos[1] = 0.0f;
    
    view = glm::mat4(1.f);
    view = glm::rotate(view, cameraAngleUpDown , glm::vec3(1.f, 0.f, 0.f));
    view = glm::rotate(view, cameraAngleZ, glm::vec3(0.f, 1.f, 0.f));
    view = glm::translate(view, cameraPos);

    viewProj = proj * view;
}

void updateViewProj(GLuint program, bool viewProjTogether)
{
    if(viewProjTogether)
    {
        auto viewProjL = glGetUniformLocation(program, "viewProj");
        glProgramUniformMatrix4fv(program, viewProjL, 1, GL_FALSE, (float*)&viewProj);
    }
    else
    {
        auto viewL = glGetUniformLocation(program, "view");
        auto projL = glGetUniformLocation(program, "proj");
    
        glProgramUniformMatrix4fv(program, projL, 1, GL_FALSE, (float*)&proj);
        glProgramUniformMatrix4fv(program, viewL, 1, GL_FALSE, (float*)&view);
    }

    cameraChangeStateForLod = true;
}


bool handleControls()
{
    SDL_Event event;
    bool updatePosition = false;
    
    // Event loop
    while (SDL_PollEvent(&event))
    {
        

        if (event.type == SDL_EVENT_QUIT) return false; // tell main loop to quit
        if (event.type == SDL_EVENT_KEY_DOWN) 
        {
            keys[event.key.key] = true;
        }
        if (event.type == SDL_EVENT_KEY_UP) {
            keys[event.key.key] = false;
        }

        if (event.type == SDL_EVENT_MOUSE_MOTION)
        {
            if (event.motion.state & SDL_BUTTON_LEFT)
            {
                updatePosition = true;
                cameraUpdateAngle(event.motion.xrel, event.motion.yrel);
            }
        }
    }
    // update camera speed
    if(keys[SDLK_W] || keys[SDLK_S] || keys[SDLK_LSHIFT] || keys[SDLK_SPACE] || keys[SDLK_A] || keys[SDLK_D])
    {
        updatePosition = true;
        cameraSpeed += 0.005f;
    }
    else
    {
        cameraSpeed = 1.f;
    }

    if(updatePosition)
        cameraUpdatePosition();

    return true;
}


void camera_init()
{
    cameraUpdatePosition();
}