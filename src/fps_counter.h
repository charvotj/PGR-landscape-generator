#pragma once

#include <imgui.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_opengl3.h>

void fpsCounter_init(SDL_Window *window, void *sdl_gl_context);
void fpsCounter_deinit();
void fpsCounter_tick();