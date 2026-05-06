#define SDL_MAIN_USE_CALLBACKS 1;
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

#include <string>
#include <utility>
#include <vector>
#include <chrono>
#include <format>

using datatime = std::chrono::system_clock::time_point;

class Message {
private:
    inline static int s_NextID = 0;
    int m_ID;
    datatime m_DataTimeSent;
    std::string m_Content;
    std::string m_Author;
public:
    Message(std::string m_Content, std::string m_Author) : m_Content(std::move(m_Content)), m_Author(std::move(m_Author)) {
        this->m_DataTimeSent = std::chrono::system_clock::now();
        this->m_ID = s_NextID++;
    }

    std::string GetDataTime() {
        return std::format("{:%Y-%m-%d %H:%M:%S}", this->m_DataTimeSent);
    }

    std::string& GetAuthor() {
        return this->m_Author;
    }

    std::string& GetContent() {
        return this->m_Content;
    }

    int GetID() const {
        return this->m_ID;
    }
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
