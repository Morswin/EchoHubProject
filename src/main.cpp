#define SDL_MAIN_USE_CALLBACKS 1;
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

#include <string>
#include <vector>
#include <chrono>
#include <format>

#include "message.hpp"


/**
 * @brief A way to describe a view state in 1 enum.
 *
 * This doesn't include the data, and that's why ViewState class exists.
 * I'll use this docstring for planning out the initial set of views.
 * TODO: Remove this part of docstring once the initial set of views is implemented. This part of docstring may not be needed by then.
 *
 * LANDING_VIEW:
 *  Whenever you open the app, this is the entry view for the program.
 * CONNECT_TO_NEW_SERVER_VIEW:
 *  Here user may provide data required for a connecting to an already existing server.
 * CREATE_NEW_SERVER_VIEW:
 *  Here user may set up a new server.
 * SERVER_VIEW:
 *  Displays relevant info about server that the user is currently connected to.
 *  Contains channel selection section and chat section (if a text channel has been selected)
 * USER_SETTINGS_VIEW:
 *  Here user should be able to define or configure their profile.
 * SERVER_SETTINGS_VIEW:
 *  Here a server owner will be able to configure server settings.
 */
enum class EViewState {
    LANDING_VIEW,
    CONNECT_TO_NEW_SERVER_VIEW,
    CREATE_NEW_SERVER,
    SERVER_VIEW,
    USER_SETTINGS_VIEW,
    SERVER_SETTINGS_VIEW,
};

/**
 * @brief This is a container for data regarding a view state.
 *
 * Depending on a EViewState, ViewStateManager will be expecting different fields to be filled with data.
 */
class ViewState {
private:
    EViewState m_EViewState;
public:

};

/**
 * @brief This is supposed to contain what view should the app be showing at a given moment.
 *
 * This should be the sole and only source of truth about the view state.
 */
class ViewStateManager {
private:
    ViewState m_ViewState;
public:

};

SDL_Window *window = nullptr;
SDL_Renderer *renderer = nullptr;

std::vector<Message> messages = {
    {"Message 1", "User 1"},
    {"Message 2", "User 2"},
    {"Message 3", "User 1"},
    {"Message 4", "User 2"},
    {"Message 5", "User 3"},
    {"Message 6", "User 1"},
    {"Message 7", "User 2"},
};
static char message_buffer[1024] = "";

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
            if (ImGui::BeginChild("Messages", ImVec2(0.0f, -60.0f), false)) {
                for (Message& message : messages) {
                    if (ImGui::BeginChild(std::format("Message-{}", message.GetID()).c_str(), ImVec2(0.0f, 50.0f), true)) {
                        ImGui::Text("%s", message.GetAuthor().c_str());
                        ImGui::SameLine();
                        ImGui::Text("%s", message.GetDataTime().c_str());
                        ImGui::Text("%s", message.GetContent().c_str());
                    } ImGui::EndChild();
                }
            }
            ImGui::EndChild();
            if (ImGui::BeginChild("NewMessageInput", ImVec2(0.0f, 45), false)) {
                if (ImGui::InputText("##message_input", message_buffer, IM_ARRAYSIZE(message_buffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
                    // Message is being sent
                    SDL_Log("%s", message_buffer);
                    message_buffer[0] = '\0';
                }
                ImGui::SameLine();
                if (ImGui::Button("Send")) {
                    SDL_Log("%s", message_buffer);
                }
            } ImGui::EndChild();
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
