#ifndef ECHOHUBPROJECT_APP_STATE_HPP
#define ECHOHUBPROJECT_APP_STATE_HPP

#include <SDL3/SDL.h>
#include <vector>
#include <string>
#include <memory>
#include <functional>
#include "ui/theme.hpp"
#include "ui/molecules.hpp"
#include "ui/views.hpp"
#include "network/server.hpp"
#include "network/client.hpp"
#include "voice/voice_client.hpp"
#include "utils/thread_safe_queue.hpp"

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
    
    // --- Saved Servers ---
    std::vector<std::string> savedServers;
    
    // --- Channel Creation ---
    std::string newChannelName;
    bool newChannelIsText = true;

    // --- Chat Data ---
    std::string chatInput;
    std::string dmChatInput;
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
    std::vector<Molecules::Message> dmMessages;
    std::string currentDMFriend;
    
    // --- Channel Users ---
    std::vector<std::string> usersInChannel;
    std::string currentUserListChannel;
    std::unordered_map<std::string, std::vector<std::string>> usersInVoiceChannels;

    // --- Theme ---
    Theme theme = Theme::createGoldGalaxyTheme();

    // --- Network ---
    std::unique_ptr<Network::Server> server;
    std::unique_ptr<Network::Client> client;
    std::unique_ptr<VoiceClient> voiceClient;
    bool isServerRunning = false;
    bool isConnectedToServer = false;
    bool isVoiceActive = false;
    bool isConnecting = false;
    std::string connectionStatus = "";
    std::string currentChannel = "ogólny"; // Current text channel
    std::string currentVoiceChannel = "";
    
    // --- Main Thread Callback Queue ---
    ThreadSafeQueue<std::function<void()>> mainThreadCallbacks;
    
    // --- Connection Timeout ---
    std::chrono::steady_clock::time_point connectionStartTime;
    static constexpr std::chrono::seconds CONNECTION_TIMEOUT = std::chrono::seconds(5);

    // --- Network Callbacks ---
    void setupNetworkCallbacks();
    void processMainThreadCallbacks();
    void onTextMessageReceived(const Network::TextMessage& msg);
    void onVoicePacketReceived(const Network::VoicePacket& pkt);
    void onConnectionStatusChanged(bool connected, const std::string& message);
    void onChannelListReceived(const std::vector<Network::ChannelInfo>& channels);
    void onUserListReceived(const std::vector<std::string>& users);
    void onVoiceUserListReceived(const std::string& channel, const std::vector<std::string>& users);
    
    // --- Channel Controls ---
    void switchChannel(const std::string& channel, bool isTextChannel);
    void createChannel(const std::string& channelName, bool isTextChannel);
    
    // --- Voice Controls ---
    void startVoice(const std::string& channel);
    void stopVoice();
    void toggleVoice(const std::string& channel);

    // --- Server Management ---
    void addSavedServer(const std::string& serverName) {
        if (std::find(savedServers.begin(), savedServers.end(), serverName) == savedServers.end()) {
            savedServers.push_back(serverName);
        }
    }
    
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
    [[nodiscard]] std::string& getDMChatInput() { return dmChatInput; }
    [[nodiscard]] std::vector<Molecules::Friend>& getFriends() { return friends; }
    [[nodiscard]] std::vector<Views::Channel>& getChannels() { return channels; }
    [[nodiscard]] std::vector<Molecules::Message>& getMessages() { return messages; }
    [[nodiscard]] std::vector<Molecules::Message>& getDMMessages() { return dmMessages; }
    [[nodiscard]] const std::string& getCurrentDMFriend() const { return currentDMFriend; }
    void setCurrentDMFriend(const std::string& friendName) { currentDMFriend = friendName; }
    [[nodiscard]] std::string& getSelectedMic() { return selectedMic; }
    [[nodiscard]] std::string& getVerificationLevel() { return verificationLevel; }

    [[nodiscard]] EViewState getView() const { return currentView; }
    void setView(EViewState view) { currentView = view; }
    
    [[nodiscard]] bool getIsVoiceActive() const { return isVoiceActive; }
    [[nodiscard]] const std::string& getCurrentVoiceChannel() const { return currentVoiceChannel; }
    [[nodiscard]] const std::string& getCurrentChannel() const { return currentChannel; }
    [[nodiscard]] const std::string& getCurrentVoiceChannelRef() const { return currentVoiceChannel; }
    
    void setCurrentChannel(const std::string& channel) { currentChannel = channel; }
    void setCurrentVoiceChannel(const std::string& channel) { currentVoiceChannel = channel; }
    
    [[nodiscard]] const std::vector<std::string>& getUsersInChannel() const { return usersInChannel; }
    void setUsersInChannel(const std::vector<std::string>& users) { usersInChannel = users; }
    
    [[nodiscard]] const std::unordered_map<std::string, std::vector<std::string>>& getUsersInVoiceChannels() const { return usersInVoiceChannels; }
    void setUsersInVoiceChannel(const std::string& channel, const std::vector<std::string>& users) {
        usersInVoiceChannels[channel] = users;
    }
    
    [[nodiscard]] bool hasOtherUsersInVoiceChannel() const {
        auto it = usersInVoiceChannels.find(currentVoiceChannel);
        if (it == usersInVoiceChannels.end()) {
            return false; // No info about this channel
        }
        // Check if there are users other than ourselves
        return it->second.size() > 1 || (it->second.size() == 1 && it->second[0] != username);
    }
    
    [[nodiscard]] bool getIsConnecting() const { return isConnecting; }
    void setIsConnecting(bool connecting) { isConnecting = connecting; }
    
    [[nodiscard]] const std::vector<std::string>& getSavedServers() const { return savedServers; }
};

#endif //ECHOHUBPROJECT_APP_STATE_HPP