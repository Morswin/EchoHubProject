#include "app_state.hpp"
#include <chrono>
#include <iomanip>
#include <sstream>

// Helper function to get current timestamp
std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&time);
    
    std::ostringstream oss;
    oss << std::put_time(&tm, "%H:%M:%S");
    return oss.str();
}

// --- Network Callback Implementations ---

void AppState::setupNetworkCallbacks() {
    if (client) {
        // Set up client callbacks
        client->setTextMessageCallback([this](const Network::TextMessage& msg) {
            onTextMessageReceived(msg);
        });
        
        client->setVoicePacketCallback([this](const Network::VoicePacket& pkt) {
            onVoicePacketReceived(pkt);
        });
        
        client->setConnectionCallback([this](bool connected, const std::string& message) {
            onConnectionStatusChanged(connected, message);
        });
        
        client->setChannelListCallback([this](const std::vector<Network::ChannelInfo>& channels) {
            onChannelListReceived(channels);
        });
        
        client->setUserListCallback([this](const std::vector<std::string>& users) {
            onUserListReceived(users);
        });
    }
}

void AppState::onTextMessageReceived(const Network::TextMessage& msg) {
    // Add the message to the messages vector
    messages.push_back({
        msg.author,
        msg.content,
        msg.timestamp.empty() ? getCurrentTimestamp() : msg.timestamp
    });
    
    // Update connection status
    connectionStatus = "Message received from " + msg.author;
}

void AppState::onVoicePacketReceived(const Network::VoicePacket& pkt) {
    // Handle voice packet (pass to voice client for playback)
    if (voiceClient) {
        voiceClient->queueVoicePacket(pkt.audioData);
    }
    
    connectionStatus = "Voice packet received from " + pkt.sender;
}

void AppState::onConnectionStatusChanged(bool connected, const std::string& message) {
    isConnectedToServer = connected;
    connectionStatus = message;
    
    if (connected) {
        // Connection successful - transition to server view
        currentView = EViewState::SERVER_VIEW;
    } else {
        // Connection failed or lost
        if (message.find("failed") != std::string::npos || message.find("error") != std::string::npos) {
            currentView = EViewState::ERROR_DISCONNECTED_VIEW;
        }
    }
}

void AppState::onChannelListReceived(const std::vector<Network::ChannelInfo>& channelInfos) {
    // Update the channels list
    channels.clear();
    for (const auto& channelInfo : channelInfos) {
        channels.push_back({
            channelInfo.name,
            channelInfo.icon,
            channelInfo.isTextChannel
        });
    }
}

void AppState::onUserListReceived(const std::vector<std::string>& users) {
    // Update users in current channel
    setUsersInChannel(users);
    
    // Also update friends list
    friends.clear();
    for (const auto& user : users) {
        friends.push_back({user, true}); // All users are online when received from server
    }
}

// --- Channel Controls ---

void AppState::switchChannel(const std::string& channel, bool isTextChannel) {
    if (!client || !client->isConnected()) {
        return;
    }
    
    // Leave current voice channel if switching to a different one
    if (isTextChannel && isVoiceActive && currentVoiceChannel != channel) {
        stopVoice();
    }
    
    // Set new current channel
    setCurrentChannel(channel);
    
    // Join the channel on the server
    client->joinChannel(channel);
    
    // Request user list for the new channel
    client->requestUserList();
    
    // If it's a voice channel and we're not already in voice, auto-join
    if (!isTextChannel && !isVoiceActive) {
        startVoice(channel);
    }
    
    connectionStatus = "Switched to channel: " + channel;
}

void AppState::createChannel(const std::string& channelName, bool isTextChannel) {
    if (!server || !server->isRunning()) {
        connectionStatus = "Error: Server not running";
        return;
    }
    
    // Add channel to server
    server->addChannel(channelName, isTextChannel ? "#" : "🔊", isTextChannel);
    
    // If this is the server we're connected to, also add to our local channels
    if (client && client->isConnected()) {
        // Request updated channel list
        client->requestChannelList();
    }
    
    connectionStatus = "Created channel: " + channelName;
    newChannelName.clear();
}

// --- Voice Controls ---

void AppState::startVoice(const std::string& channel) {
    if (isVoiceActive || !client || !client->isConnected()) {
        return;
    }
    
    // Initialize voice client if not already done (MUST be done in main thread)
    if (!voiceClient) {
        voiceClient = std::make_unique<VoiceClient>();
        if (!voiceClient->initialize()) {
            connectionStatus = "Error: Failed to initialize voice client";
            voiceClient.reset();
            return;
        }
    }
    
    // Set the voice channel
    setCurrentVoiceChannel(channel);
    client->setVoiceChannel(channel);
    
    // Start voice client with callback to send packets via network
    isVoiceActive = voiceClient->start([this](const std::vector<uint8_t>& packet) {
        if (client && client->isConnected()) {
            client->sendVoicePacketUdp(packet);
        }
    });
    
    if (isVoiceActive) {
        connectionStatus = "Voice active in channel: " + channel;
    }
}

void AppState::stopVoice() {
    if (!isVoiceActive || !voiceClient) {
        return;
    }
    
    voiceClient->stop();
    isVoiceActive = false;
    setCurrentVoiceChannel("");
    connectionStatus = "Voice stopped";
}

void AppState::toggleVoice(const std::string& channel) {
    if (isVoiceActive && currentVoiceChannel == channel) {
        stopVoice();
    } else if (!isVoiceActive) {
        startVoice(channel);
    } else {
        // Switch voice channel
        stopVoice();
        startVoice(channel);
    }
}
