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
#include <chrono>
#include <iomanip>
#include <sstream>

#include "app_state.hpp"
#include "message.hpp"
#include "voice/voice_client.hpp"
#include "network/server.hpp"
#include "network/client.hpp"
#include "ui/views.hpp"
#include "view_states.hpp"

// Helper function to get current timestamp
std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&time);
    
    std::ostringstream oss;
    oss << std::put_time(&tm, "%H:%M:%S");
    return oss.str();
}


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

    // Process callbacks from background threads (network, voice, etc.)
    g_AppState.processMainThreadCallbacks();

    // --- 5. System przełączania widoków ---
    switch (g_AppState.getView()) {
        case EViewState::AUTH_VIEW:
            Views::AuthView(g_AppState.getUsername(), g_AppState.getTheme(),
                [&]() { g_AppState.setView(EViewState::LANDING_VIEW); },  // Zaloguj
                [&]() { g_AppState.setView(EViewState::REGISTER_VIEW); }  // Zarejestruj
            );
            break;

        case EViewState::LANDING_VIEW:
            Views::LandingView(g_AppState.getUsername(), g_AppState.getTheme(),
                [&]() { g_AppState.setView(EViewState::FRIENDS_LIST_VIEW); },
                [&]() { g_AppState.setView(EViewState::CONNECT_TO_NEW_SERVER_VIEW); },
                [&]() { 
                    std::cout << "[LANDING] Navigating to CREATE_NEW_SERVER_VIEW" << std::endl;
                    g_AppState.setView(EViewState::CREATE_NEW_SERVER_VIEW);
                },
                [&]() { g_AppState.setView(EViewState::USER_SETTINGS_VIEW); }
            );
            break;

        case EViewState::FRIENDS_LIST_VIEW:
            Views::FriendsListView(g_AppState.getFriends(), g_AppState.getTheme(),
                [&]() { g_AppState.setView(EViewState::CREATE_NEW_SERVER_VIEW); },
                [&](const std::string& serverId) {
                    // Connect to server callback (for saved servers)
                    g_AppState.setView(EViewState::CONNECT_TO_NEW_SERVER_VIEW);
                },
                [&]() {
                    // DM button - stay in FriendsListView (do nothing or just stay)
                    // This is the direct message button, should not connect to server
                },
                [&](const std::string& friendName) {
                    // Start DM chat with this friend
                    g_AppState.setCurrentDMFriend(friendName);
                    g_AppState.setView(EViewState::DIRECT_MESSAGE_VIEW);
                }
            );
            break;

        case EViewState::DIRECT_MESSAGE_VIEW:
            Views::DirectMessageView(g_AppState.getCurrentDMFriend(), g_AppState.getDMMessages(), 
                g_AppState.getDMChatInput(), g_AppState.getTheme(),
                [&](const std::string& msg) {
                    // Send DM message (local echo for now)
                    g_AppState.getDMMessages().push_back({
                        g_AppState.getCurrentDMFriend(),
                        msg,
                        getCurrentTimestamp()
                    });
                    g_AppState.getDMChatInput().clear();
                },
                [&]() {
                    // Back to friends list
                    g_AppState.setView(EViewState::FRIENDS_LIST_VIEW);
                }
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
                    g_AppState.setIsConnecting(true);
                    g_AppState.connectionStartTime = std::chrono::steady_clock::now();
                    bool connected = g_AppState.client->connect(username, "password");
                    
                    if (connected) {
                        g_AppState.setView(EViewState::CONNECTING_LOADING_VIEW);
                    } else {
                        g_AppState.connectionStatus = "Connection failed";
                        g_AppState.setIsConnecting(false);
                        g_AppState.setView(EViewState::ERROR_DISCONNECTED_VIEW);
                    }
                },
                [&]() { g_AppState.setView(EViewState::LANDING_VIEW); }
            );
            break;

        case EViewState::CREATE_NEW_SERVER_VIEW:
            Views::CreateNewServerView(g_AppState.getServerName(), g_AppState.getTheme(),
                [&]() { 
                    // Check if server name is empty
                    if (g_AppState.getServerName().empty()) {
                        std::cerr << "[SERVER CREATION] ERROR: Server name cannot be empty!" << std::endl;
                        g_AppState.connectionStatus = "Server name cannot be empty";
                        g_AppState.setView(EViewState::LANDING_VIEW);
                        return;
                    }
                    std::cout << "[SERVER CREATION] Step 1: Stopping existing server if running..." << std::endl;
                    // Create server and connect to it
                    if (g_AppState.server && g_AppState.server->isRunning()) {
                        std::cout << "[SERVER CREATION] Stopping existing server..." << std::endl;
                        g_AppState.server->stop();
                        std::cout << "[SERVER CREATION] Existing server stopped." << std::endl;
                    }
                    
                    std::cout << "[SERVER CREATION] Step 2: Creating new Server instance with name: " << g_AppState.getServerName() << std::endl;
                    g_AppState.server = std::make_unique<Network::Server>(9987, 9988);
                    std::cout << "[SERVER CREATION] Server instance created." << std::endl;
                    
                    std::cout << "[SERVER CREATION] Step 3: Adding default channels..." << std::endl;
                    // Add default channels BEFORE starting the server (2 text + 1 voice)
                    g_AppState.server->addChannel("ogólny", "#", true);
                    std::cout << "[SERVER CREATION] Added channel: ogólny" << std::endl;
                    g_AppState.server->addChannel("pomoc-kod", "#", true);
                    std::cout << "[SERVER CREATION] Added channel: pomoc-kod" << std::endl;
                    g_AppState.server->addChannel("Poczekalnia", "🔊", false);
                    std::cout << "[SERVER CREATION] Added channel: Poczekalnia" << std::endl;
                    std::cout << "[SERVER CREATION] All default channels added (2 text + 1 voice)." << std::endl;
                    
                    std::cout << "[SERVER CREATION] Step 4: Starting server..." << std::endl;
                    g_AppState.isServerRunning = g_AppState.server->start();
                    std::cout << "[SERVER CREATION] Server start() returned: " << (g_AppState.isServerRunning ? "true" : "false") << std::endl;
                    
                    if (g_AppState.isServerRunning) {
                        std::cout << "[SERVER CREATION] Step 5: Server started successfully. Creating client..." << std::endl;
                        // Auto-connect to our own server
                        g_AppState.client = std::make_unique<Network::Client>("127.0.0.1", 9987, 9988);
                        std::cout << "[SERVER CREATION] Client instance created." << std::endl;
                        
                        g_AppState.setupNetworkCallbacks();
                        std::cout << "[SERVER CREATION] Network callbacks set up." << std::endl;
                        
                        std::string username = g_AppState.getUsername().empty() ? g_AppState.nickname : g_AppState.getUsername();
                        std::cout << "[SERVER CREATION] Using username: " << username << std::endl;
                        g_AppState.setIsConnecting(true);
                        g_AppState.connectionStartTime = std::chrono::steady_clock::now();
                        
                        std::cout << "[SERVER CREATION] Step 6: Connecting client to server..." << std::endl;
                        bool connected = g_AppState.client->connect(username, "password");
                        std::cout << "[SERVER CREATION] Client connect() returned: " << (connected ? "true" : "false") << std::endl;
                        
                        if (connected) {
                            std::cout << "[SERVER CREATION] SUCCESS: Server created and client connected!" << std::endl;
                            g_AppState.setView(EViewState::CONNECTING_LOADING_VIEW);
                        } else {
                            std::cerr << "[SERVER CREATION] ERROR: Client connection failed!" << std::endl;
                            g_AppState.connectionStatus = "Connection failed";
                            g_AppState.setIsConnecting(false);
                            g_AppState.setView(EViewState::ERROR_DISCONNECTED_VIEW);
                        }
                    } else {
                        std::cerr << "[SERVER CREATION] ERROR: Server failed to start!" << std::endl;
                    }
                },
                [&]() { g_AppState.setView(EViewState::LANDING_VIEW); }
            );
            break;

        case EViewState::SERVER_VIEW:
            Views::ServerView(g_AppState.getServerName().empty() ? "Mój Serwer C++" : g_AppState.getServerName(), g_AppState.getChannels(), g_AppState.getMessages(), g_AppState.getChatInput(), g_AppState.getTheme(),
                g_AppState.getCurrentChannel(),
                g_AppState.getCurrentVoiceChannelRef(),
                g_AppState.getIsVoiceActive(),
                g_AppState.getUsersInChannel(),
                g_AppState.getUsersInVoiceChannels(),
                g_AppState.getSavedServers(),
                [&](const std::string& msg) {
                    // Send message through network client if connected
                    if (g_AppState.client && g_AppState.client->isConnected()) {
                        g_AppState.client->sendTextMessage(g_AppState.getCurrentChannel(), msg);
                        // Local echo - add message immediately to UI
                        g_AppState.getMessages().push_back({
                            g_AppState.getCurrentChannel(),
                            g_AppState.getUsername(),
                            msg,
                            getCurrentTimestamp()
                        });
                    } else {
                        // Local echo for testing
                        g_AppState.getMessages().push_back({
                            g_AppState.getCurrentChannel(),
                            g_AppState.getUsername(),
                            msg,
                            getCurrentTimestamp()
                        });
                    }
                    g_AppState.getChatInput().clear();
                },
                [&]() { g_AppState.setView(EViewState::CREATE_NEW_SERVER_VIEW); },
                [&](const std::string& serverId) { 
                    // Connect to existing server
                    g_AppState.setView(EViewState::CONNECT_TO_NEW_SERVER_VIEW); 
                },
                [&]() { g_AppState.setView(EViewState::FRIENDS_LIST_VIEW); },
                [&](const std::string& channel, bool isTextChannel) {
                    // Switch channel
                    g_AppState.switchChannel(channel, isTextChannel);
                },
                [&]() {
                    // Create new channel
                    g_AppState.setView(EViewState::CREATE_CHANNEL_VIEW);
                },
                [&]() {
                    // Disconnect from voice channel
                    g_AppState.stopVoice();
                }
            );
            break;

        case EViewState::USER_SETTINGS_VIEW:
            Views::UserSettingsView(g_AppState.getUsername(), g_AppState.getSelectedMic(), g_AppState.getTheme(),
                [&]() { g_AppState.setView(EViewState::LANDING_VIEW); },
                [&]() {
                    // Logout - clear username and go back to auth
                    g_AppState.setUsername("");
                    g_AppState.setView(EViewState::AUTH_VIEW);
                }
            );
            break;

        case EViewState::SERVER_SETTINGS_VIEW:
            Views::ServerSettingsView(g_AppState.getServerName(), g_AppState.getVerificationLevel(), g_AppState.getTheme(),
                [&]() { g_AppState.setView(EViewState::SERVER_VIEW); }
            );
            break;

        case EViewState::CONNECTING_LOADING_VIEW: { // <--- DODAJ TĘ KLAMRĘ
            Views::ConnectingLoadingView(g_AppState.getTheme(), g_AppState.connectionStatus);
            // If connected, transition to server view
            if (g_AppState.isConnectedToServer) {
                g_AppState.setView(EViewState::SERVER_VIEW);
                g_AppState.setIsConnecting(false);
            }
            // Check for connection timeout (5 seconds)
            else if (g_AppState.getIsConnecting()) {
                auto now = std::chrono::steady_clock::now(); // Pierwsza zmienna
                if (now - g_AppState.connectionStartTime > AppState::CONNECTION_TIMEOUT) {
                    g_AppState.connectionStatus = "Connection timeout: Server not responding";
                    g_AppState.setIsConnecting(false);
                    g_AppState.setView(EViewState::ERROR_DISCONNECTED_VIEW);
                }
            }
            // Fallback: auto-transition after delay if connection status hasn't been updated
            static int counter = 0; // Druga zmienna
            if (counter++ > 120) { // Po 120 klatkach (ok. 2s)
                if (g_AppState.client && g_AppState.client->isConnected()) {
                    g_AppState.setView(EViewState::SERVER_VIEW);
                    g_AppState.setIsConnecting(false);
                } else if (g_AppState.isServerRunning) {
                    // Server is running, try to connect locally
                    g_AppState.client = std::make_unique<Network::Client>("127.0.0.1", 9987, 9988);
                    g_AppState.setupNetworkCallbacks();
                    std::string username = g_AppState.getUsername().empty() ? g_AppState.nickname : g_AppState.getUsername(); // Trzecia zmienna
                    g_AppState.setIsConnecting(true);
                    g_AppState.connectionStartTime = std::chrono::steady_clock::now();
                    g_AppState.client->connect(username, "password");
                }
                counter = 0;
            }
            break;
        }

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

        case EViewState::CREATE_CHANNEL_VIEW:
            Views::CreateChannelView(g_AppState.newChannelName, g_AppState.newChannelIsText, g_AppState.getTheme(),
                [&](const std::string& name, bool isText) {
                    g_AppState.createChannel(name, isText);
                    g_AppState.setView(EViewState::SERVER_VIEW);
                },
                [&]() { g_AppState.setView(EViewState::SERVER_VIEW); }
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
    // Shutdown voice client BEFORE SDL_Quit to avoid mutex issues
    // Use shutdown() instead of stop() to ensure full cleanup
    if (g_AppState.voiceClient) {
        g_AppState.voiceClient->shutdown();
        g_AppState.voiceClient.reset();
    }
    
    // Stop server and client
    if (g_AppState.server) {
        g_AppState.server->stop();
        g_AppState.server.reset();
    }
    if (g_AppState.client) {
        g_AppState.client->disconnect();
        g_AppState.client.reset();
    }
    
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    SDL_DestroyRenderer(g_AppState.renderer);
    SDL_DestroyWindow(g_AppState.window);
    SDL_Quit();
}



// 