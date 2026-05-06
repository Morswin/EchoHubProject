#define SDL_MAIN_USE_CALLBACKS 1;
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

SDL_Window *window = nullptr;
SDL_Renderer *renderer = nullptr;

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
    // Initializing window and renderer with SDL3
    if (!SDL_Init(SDL_INIT_VIDEO)) return SDL_APP_FAILURE;
    if (!SDL_CreateWindowAndRenderer("EchoHub", 800, 600, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
        return SDL_APP_FAILURE;
    }
    // Initiating ImGui
    ImGui::CreateContext();
    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    // Start rendering imgui frame
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    // The actual contents of the window
    int main_window_w, main_window_h;
    SDL_GetWindowSize(window, &main_window_w, &main_window_h);
    ImGui::SetNextWindowSize(ImVec2(static_cast<float>(main_window_w), static_cast<float>(main_window_h)));
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    if (ImGui::Begin("EchoHubRoot", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar)) {
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(ImVec2(static_cast<float>(main_window_w) / 4.0f, static_cast<float>(main_window_h)));
        if (ImGui::Begin("EchoHubNavigation", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
            ImGui::Text("Navigation");
        } ImGui::End();
        ImGui::SameLine();
        ImGui::SetNextWindowPos(ImVec2((static_cast<float>(main_window_w) / 4.0f), 0.0f));
        ImGui::SetNextWindowSize(ImVec2(static_cast<float>(main_window_w) / 2.0f, static_cast<float>(main_window_h)));
        if (ImGui::Begin("EchoHubContent", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
            ImGui::Text("Content");
        } ImGui::End();
        ImGui::SameLine();
        ImGui::SetNextWindowPos(ImVec2((static_cast<float>(main_window_w) / 4.0f * 3.0f), 0.0f));
        ImGui::SetNextWindowSize(ImVec2(static_cast<float>(main_window_w) / 4.0f, static_cast<float>(main_window_h)));
        if (ImGui::Begin("EchoHubDetails", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
            ImGui::Text("Details");
        } ImGui::End();
    } ImGui::End();

    // Rendering background
    SDL_SetRenderDrawColor(renderer, 33, 33, 33, 255);
    SDL_RenderClear(renderer);

    // Render the ImGui
    ImGui::Render();
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);

    // Display the frame
    SDL_RenderPresent(renderer);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    // Passing events to ImGui so the UI can be interacted with
    ImGui_ImplSDL3_ProcessEvent(event);

    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;  // Exits the app
    }
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
