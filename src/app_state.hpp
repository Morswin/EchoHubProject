#ifndef ECHOHUBPROJECT_APP_STATE_HPP
#define ECHOHUBPROJECT_APP_STATE_HPP

#include <SDL3/SDL.h>
#include <vector>
#include <string>
#include <memory>
#include "ui/theme.hpp"
#include "ui/molecules.hpp"
#include "ui/views.hpp"
#include "network/server.hpp"
#include "network/client.hpp"
#include "voice/voice_client.hpp"

class AppState {
public:
    // --- SDL ---
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    // --- View State ---
    EViewState currentView = EViewState::AUTH_VIEW;

    // --- User Data ---
    std::string username;
    std::string email;
    std::string password;
    std::string confirmPassword;

    // --- Server Data ---
    std::string serverAddress;
    std::string nickname;
    std::string serverName;
    std::string region;
    std::string selectedMic = "Default Microphone Input";
    std::string verificationLevel = "Brak (Dowolny użytkownik może wejść)";

    // --- Chat Data ---
    std::string chatInput;
    std::vector<Molecules::Friend> friends = {
        {"JanKowalski", true},
        {"Programista99", false}
    };
    std::vector<Views::Channel> channels = {
        {"ogólny", "#", true},
        {"pomoc-kod", "#", true},
        {"Poczekalnia", "🔊", false},
        {"Pokój gier", "🔊", false}
    };
    std::vector<Molecules::Message> messages = {
        {"Admin", "Cześć! Witamy na klonie Discorda napisanym w C++.", "Dzisiaj o 12:00"},
        {"User_C++", "Wygląda super, czat tekstowy i enumy działają stabilnie!", "Dzisiaj o 12:05"}
    };

    // --- Theme ---
    Theme theme = Theme::createGoldGalaxyTheme();

    // --- Network ---
    std::unique_ptr<Network::Server> server;
    std::unique_ptr<Network::Client> client;
    std::unique_ptr<VoiceClient> voiceClient;
    bool isServerRunning = false;
    bool isConnectedToServer = false;
    bool isVoiceActive = false;
    std::string connectionStatus = "";
    std::string currentVoiceChannel = "";

    // --- Network Callbacks ---
    void setupNetworkCallbacks();
    void onTextMessageReceived(const Network::TextMessage& msg);
    void onVoicePacketReceived(const Network::VoicePacket& pkt);
    void onConnectionStatusChanged(bool connected, const std::string& message);
    void onChannelListReceived(const std::vector<Network::ChannelInfo>& channels);
    void onUserListReceived(const std::vector<std::string>& users);
    
    // --- Voice Controls ---
    void startVoice(const std::string& channel);
    void stopVoice();
    void toggleVoice(const std::string& channel);

    // --- Gettery/Settery ---
    [[nodiscard]] EViewState getCurrentView() const { return currentView; }
    void setCurrentView(EViewState view) { currentView = view; }

    [[nodiscard]] const std::string& getUsername() const { return username; }
    void setUsername(const std::string& newUsername) { username = newUsername; }

    [[nodiscard]] SDL_Window* getWindow() const { return window; }
    [[nodiscard]] SDL_Renderer* getRenderer() const { return renderer; }
    [[nodiscard]] Theme& getTheme() { return theme; }
    [[nodiscard]] std::string& getServerAddress() { return serverAddress; }
    [[nodiscard]] std::string& getUsername() { return username; }
    [[nodiscard]] std::string& getServerName() { return serverName; }
    [[nodiscard]] std::string& getRegion() { return region; }
    [[nodiscard]] std::string& getNickname() { return nickname; }
    [[nodiscard]] std::string& getChatInput() { return chatInput; }
    [[nodiscard]] std::vector<Molecules::Friend>& getFriends() { return friends; }
    [[nodiscard]] std::vector<Views::Channel>& getChannels() { return channels; }
    [[nodiscard]] std::vector<Molecules::Message>& getMessages() { return messages; }
    [[nodiscard]] std::string& getSelectedMic() { return selectedMic; }
    [[nodiscard]] std::string& getVerificationLevel() { return verificationLevel; }

    [[nodiscard]] EViewState getView() const { return currentView; }
    void setView(EViewState view) { currentView = view; }
    
    [[nodiscard]] bool getIsVoiceActive() const { return isVoiceActive; }
    [[nodiscard]] const std::string& getCurrentVoiceChannel() const { return currentVoiceChannel; }
};

#endif //ECHOHUBPROJECT_APP_STATE_HPP