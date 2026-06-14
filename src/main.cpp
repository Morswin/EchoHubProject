#define SDL_MAIN_USE_CALLBACKS 1;
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>
#include <opus.h>

#include <string>
#include <vector>
#include <format>
#include <iostream>

#include "app_state.hpp"
#include "message.hpp"
#include "voice/voice_client.hpp"
#include "network/server.hpp"
#include "network/client.hpp"
#include "ui/views.hpp"
#include "view_states.hpp"


// Global app state
AppState g_AppState;


SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
    // Initializing window and renderer with SDL3
    if (!SDL_Init(SDL_INIT_VIDEO)) return SDL_APP_FAILURE;
    if (!SDL_CreateWindowAndRenderer("EchoHub", 800, 600, SDL_WINDOW_RESIZABLE, &g_AppState.window, &g_AppState.renderer)) {
        return SDL_APP_FAILURE;
    }
    // Initiating ImGui
    ImGui::CreateContext();
    ImGui_ImplSDL3_InitForSDLRenderer(g_AppState.getWindow(), g_AppState.getRenderer());
    ImGui_ImplSDLRenderer3_Init(g_AppState.getRenderer());

    // 1. Załaduj fonty (MUSI BYĆ PRZED PIERWSZYM ImGui::NewFrame!)
    ImGuiIO& io = ImGui::GetIO();

    // Roboto (tekst)
    g_AppState.getTheme().fontRegular = io.Fonts->AddFontFromFileTTF("assets/fonts/Roboto-Black.ttf", 16.0f);
    g_AppState.getTheme().fontBold = io.Fonts->AddFontFromFileTTF("assets/fonts/Roboto-Bold.ttf", 16.0f);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    // Start rendering imgui frame
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    // --- 5. System przełączania widoków ---
    switch (g_AppState.getView()) {
        case EViewState::AUTH_VIEW:
            Views::AuthView(g_AppState.getUsername(), g_AppState.getTheme(),
                [&]() { g_AppState.setView(EViewState::LANDING_VIEW); },  // Zaloguj
                [&]() { g_AppState.setView(EViewState::REGISTER_VIEW); }  // Zarejestruj
            );
            break;

        case EViewState::LANDING_VIEW:
            Views::LandingView(g_AppState.getTheme(),
                [&]() { g_AppState.setView(EViewState::FRIENDS_LIST_VIEW); },
                [&]() { g_AppState.setView(EViewState::CONNECT_TO_NEW_SERVER_VIEW); },
                [&]() { 
                    // Stop any existing server first
                    if (g_AppState.server && g_AppState.server->isRunning()) {
                        g_AppState.server->stop();
                    }
                    g_AppState.server = std::make_unique<Network::Server>(9987, 9988);
                    g_AppState.isServerRunning = g_AppState.server->start();
                    if (g_AppState.isServerRunning) {
                        g_AppState.setView(EViewState::CONNECTING_LOADING_VIEW);
                    }
                }
            );
            break;

        case EViewState::FRIENDS_LIST_VIEW:
            Views::FriendsListView(g_AppState.getFriends(), g_AppState.getTheme(),
                [&]() { g_AppState.setView(EViewState::CREATE_NEW_SERVER_VIEW); },
                [&](const std::string& serverId) {
                    g_AppState.setView(EViewState::SERVER_VIEW);
                },
                [&]() { g_AppState.setView(EViewState::FRIENDS_LIST_VIEW); }
            );
            break;

        case EViewState::CONNECT_TO_NEW_SERVER_VIEW:
            Views::ConnectToNewServerView(g_AppState.getServerAddress(), g_AppState.getNickname(), g_AppState.getTheme(),
                [&]() { 
                    // Connect to server
                    if (g_AppState.client && g_AppState.client->isConnected()) {
                        g_AppState.client->disconnect();
                    }
                    g_AppState.client = std::make_unique<Network::Client>(g_AppState.serverAddress, 9987, 9988);
                    g_AppState.setupNetworkCallbacks();
                    
                    // Use username from auth or default
                    std::string username = g_AppState.getUsername().empty() ? g_AppState.nickname : g_AppState.getUsername();
                    bool connected = g_AppState.client->connect(username, "password");
                    
                    if (connected) {
                        g_AppState.isConnectedToServer = true;
                        g_AppState.client->requestChannelList();
                        g_AppState.setView(EViewState::CONNECTING_LOADING_VIEW);
                    } else {
                        g_AppState.connectionStatus = "Connection failed";
                        g_AppState.setView(EViewState::ERROR_DISCONNECTED_VIEW);
                    }
                },
                [&]() { g_AppState.setView(EViewState::LANDING_VIEW); }
            );
            break;

        case EViewState::CREATE_NEW_SERVER_VIEW:
            Views::CreateNewServerView(g_AppState.getServerName(), g_AppState.getRegion(), g_AppState.getTheme(),
                [&]() { 
                    // Create server and connect to it
                    if (g_AppState.server && g_AppState.server->isRunning()) {
                        g_AppState.server->stop();
                    }
                    g_AppState.server = std::make_unique<Network::Server>(9987, 9988);
                    g_AppState.isServerRunning = g_AppState.server->start();
                    
                    // Add default channels
                    g_AppState.server->addChannel("ogólny", "#", true);
                    g_AppState.server->addChannel("pomoc-kod", "#", true);
                    g_AppState.server->addChannel("Poczekalnia", "🔊", false);
                    g_AppState.server->addChannel("Pokój gier", "🔊", false);
                    
                    if (g_AppState.isServerRunning) {
                        // Auto-connect to our own server
                        g_AppState.client = std::make_unique<Network::Client>("127.0.0.1", 9987, 9988);
                        g_AppState.setupNetworkCallbacks();
                        
                        std::string username = g_AppState.getUsername().empty() ? g_AppState.nickname : g_AppState.getUsername();
                        bool connected = g_AppState.client->connect(username, "password");
                        
                        if (connected) {
                            g_AppState.isConnectedToServer = true;
                            g_AppState.client->requestChannelList();
                            g_AppState.setView(EViewState::CONNECTING_LOADING_VIEW);
                        } else {
                            g_AppState.connectionStatus = "Connection failed";
                            g_AppState.setView(EViewState::ERROR_DISCONNECTED_VIEW);
                        }
                    }
                },
                [&]() { g_AppState.setView(EViewState::LANDING_VIEW); }
            );
            break;

        case EViewState::SERVER_VIEW:
            Views::ServerView("Mój Serwer C++", g_AppState.getChannels(), g_AppState.getMessages(), g_AppState.getChatInput(), g_AppState.getTheme(),
                g_AppState.getIsVoiceActive(),
                [&](const std::string& msg) {
                    // Send message through network client if connected
                    if (g_AppState.client && g_AppState.client->isConnected()) {
                        std::string channel = "ogólny"; // Default channel
                        g_AppState.client->sendTextMessage(channel, msg);
                    } else {
                        // Local echo for testing
                        g_AppState.getMessages().push_back({g_AppState.getUsername(), msg, "Teraz"});
                    }
                    g_AppState.getChatInput().clear();
                },
                [&]() { g_AppState.setView(EViewState::CREATE_NEW_SERVER_VIEW); },
                [&](const std::string& serverId) { g_AppState.setView(EViewState::SERVER_VIEW); },
                [&]() { 
                    // Disconnect from server
                    if (g_AppState.client && g_AppState.client->isConnected()) {
                        g_AppState.client->disconnect();
                        g_AppState.isConnectedToServer = false;
                    }
                    if (g_AppState.server && g_AppState.server->isRunning()) {
                        g_AppState.server->stop();
                        g_AppState.isServerRunning = false;
                    }
                    g_AppState.setView(EViewState::FRIENDS_LIST_VIEW);
                },
                [&](const std::string& channel) {
                    // Join voice channel
                    g_AppState.toggleVoice(channel);
                }
            );
            break;

        case EViewState::USER_SETTINGS_VIEW:
            Views::UserSettingsView(g_AppState.getUsername(), g_AppState.getSelectedMic(), g_AppState.getTheme(),
                [&]() { g_AppState.setView(EViewState::LANDING_VIEW); }
            );
            break;

        case EViewState::SERVER_SETTINGS_VIEW:
            Views::ServerSettingsView(g_AppState.getServerName(), g_AppState.getVerificationLevel(), g_AppState.getTheme(),
                [&]() { g_AppState.setView(EViewState::SERVER_VIEW); }
            );
            break;

        case EViewState::CONNECTING_LOADING_VIEW:
            Views::ConnectingLoadingView(g_AppState.getTheme());
            // If connected, transition to server view
            if (g_AppState.isConnectedToServer) {
                g_AppState.setView(EViewState::SERVER_VIEW);
            }
            // Fallback: auto-transition after delay if connection status hasn't been updated
            static int counter = 0;
            if (counter++ > 120) { // Po 120 klatkach (ok. 2s)
                if (g_AppState.client && g_AppState.client->isConnected()) {
                    g_AppState.setView(EViewState::SERVER_VIEW);
                } else if (g_AppState.isServerRunning) {
                    // Server is running, try to connect locally
                    g_AppState.client = std::make_unique<Network::Client>("127.0.0.1", 9987, 9988);
                    g_AppState.setupNetworkCallbacks();
                    std::string username = g_AppState.getUsername().empty() ? g_AppState.nickname : g_AppState.getUsername();
                    g_AppState.client->connect(username, "password");
                }
                counter = 0;
            }
            break;

        case EViewState::ERROR_DISCONNECTED_VIEW:
            Views::ErrorDisconnectedView(g_AppState.getTheme(),
                [&]() { 
                    // Reconnect
                    if (g_AppState.client) {
                        std::string username = g_AppState.getUsername().empty() ? g_AppState.nickname : g_AppState.getUsername();
                        g_AppState.client->connect(username, "password");
                    }
                    g_AppState.setView(EViewState::CONNECTING_LOADING_VIEW);
                },
                [&]() { 
                    // Clean up and go back
                    if (g_AppState.client) {
                        g_AppState.client->disconnect();
                        g_AppState.client.reset();
                    }
                    g_AppState.isConnectedToServer = false;
                    g_AppState.setView(EViewState::LANDING_VIEW);
                }
            );
            break;

        case EViewState::REGISTER_VIEW:
            Views::RegisterView(g_AppState.getUsername(), g_AppState.getTheme(),
                [&]() { g_AppState.setView(EViewState::LANDING_VIEW); },  // Powrót po rejestracji
                [&]() { g_AppState.setView(EViewState::AUTH_VIEW); }       // Anuluj
            );
            break;
    }

    // Rendering background
    SDL_SetRenderDrawColor(g_AppState.getRenderer(), 33, 33, 33, 255);
    SDL_RenderClear(g_AppState.getRenderer());

    // Render the ImGui
    ImGui::Render();
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), g_AppState.getRenderer());

    // Display the frame
    SDL_RenderPresent(g_AppState.getRenderer());

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
    SDL_DestroyRenderer(g_AppState.renderer);
    SDL_DestroyWindow(g_AppState.window);
    SDL_Quit();
}



// 