#include "fps_counter.h"

#ifndef RESOURCE_DIR
#define RESOURCE_DIR "."
#endif

#ifndef SHADERS_DIR
#define SHADERS_DIR "."
#endif

void fpsCounter_init(SDL_Window *window, void *sdl_gl_context)
{
    // ImGui initialisation
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    // backends
    ImGui_ImplSDL3_InitForOpenGL(window, sdl_gl_context);
    ImGui_ImplOpenGL3_Init("#version 450");
}

void fpsCounter_deinit()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}

void drawFPSOverlay() 
{
    const float DISTANCE = 10.0f;

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 pos = ImVec2(
        viewport->Pos.x + viewport->Size.x - DISTANCE,
        viewport->Pos.y + DISTANCE
    );

    ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(1.0f,0.0f));
    ImGui::SetNextWindowBgAlpha(0.35f);

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav;

    ImGui::Begin("FPSOverlay", nullptr, flags);

    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);

    ImGui::End();
}

void fpsCounter_tick()
{
        // --- start ImGui frame ---
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
        drawFPSOverlay();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
