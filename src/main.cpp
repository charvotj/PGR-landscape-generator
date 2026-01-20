#include "main.h"

using namespace ge::gl;

#ifndef RESOURCE_DIR
#define RESOURCE_DIR "."
#endif

#ifndef SHADERS_DIR
#define SHADERS_DIR "."
#endif

#define WINDOW_WIDTH    1280
#define WINDOW_HEIGHT   720

#define _DEBUG_PRINTS




void prepareNormalRender(GLuint program)
{
    // resetViewport
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    glClearColor(0.2, 0.2, 0.2, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    programLightViewProj(program);
}


int main(int argc, char* argv[])
{
    // SDL window initialization
    auto window = SDL_CreateWindow("PGR", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
    auto ctx = SDL_GL_CreateContext(window);

    ge::gl::init();

    #ifdef DEBUG_PRINTS
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback([](GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
        fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
                (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
                type, severity, message);
    }, 0);
    #endif

    // GL Settings
    glEnable(GL_DEPTH_TEST);
    // glEnable(GL_CULL_FACE);
    // glCullFace(GL_BACK);      // nebo GL_FRONT
    // glFrontFace(GL_CCW);      // CCW = výchozí konvexe

    glDisable(GL_BLEND);
    glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);

    
    auto mainProgram = createProgram(SHADERS_DIR "vertex.vs", SHADERS_DIR "fragment.fs");
    auto skybox = createSkyBox("07");
    
    // Init custom modules
    fpsCounter_init(window, ctx);
    camera_init();
    treesInit();
    shadowMapInit();

    // Load objects and textures
    std::vector<Material> mats;
    std::vector<Object> terain;
    std::vector<Tree> trees;
    
    loadTerrain(mats, terain);
    loadTrees(mats, trees);
    
    // Pre-render shadow map
    prepareDepthMapRender();
    setLodThreshold(10.0);// all trees should be high res
    renderTrees(depthMapProgram, mats, trees);
    renderTerrain(depthMapProgram, mats, terain);

    // Set LOD settings
    setLodThreshold(0.06);
    setFrustumCulling(true);
    
    // Prepere for rendering into main window
    prepareNormalRender(mainProgram);
    
    // Main loop
    bool running = true;
    while (running)
    {
        running = handleControls();
        if(getCameraViewProjChange())
        {
            updateViewProj(mainProgram);
            updateViewProj(skybox.program, false);
        }

        // Main render pass
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderTrees(mainProgram, mats, trees);
        renderTerrain(mainProgram, mats, terain);
        renderSkybox(skybox);
        
        // Draw fps overlay
        fpsCounter_tick();
        
        SDL_GL_SwapWindow(window);
    }
    fpsCounter_deinit();
    
    SDL_GL_DestroyContext(ctx);
    SDL_DestroyWindow(window);
    return 0;
}