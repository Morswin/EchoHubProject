#include "app_state.hpp"
#include <iostream>

// --- Network Callback Implementations ---

void AppState::setupNetworkCallbacks() {
    if (client) {
        // Set up client callbacks - all callbacks are queued to run on main thread
        client->setTextMessageCallback([this](const Network::TextMessage& msg) {
            mainThreadCallbacks.push([this, msg]() { onTextMessageReceived(msg); });
        });
        
        client->setVoicePacketCallback([this](const Network::VoicePacket& pkt) {
            mainThreadCallbacks.push([this, pkt]() { onVoicePacketReceived(pkt); });
        });
        
        client->setConnectionCallback([this](bool connected, const std::string& message) {
            mainThreadCallbacks.push([this, connected, message]() { onConnectionStatusChanged(connected, message); });
        });
        
        client->setChannelListCallback([this](const std::vector<Network::ChannelInfo>& channels) {
            mainThreadCallbacks.push([this, channels]() { onChannelListReceived(channels); });
        });
        
        client->setUserListCallback([this](const std::vector<std::string>& users) {
            mainThreadCallbacks.push([this, users]() { onUserListReceived(users); });
        });

        client->setVoiceUserListCallback([this](const std::string& channel, const std::vector<std::string>& users) {
            mainThreadCallbacks.push([this, channel, users]() { onVoiceUserListReceived(channel, users); });
        });
    }
}

void AppState::processMainThreadCallbacks() {
    // Process all pending callbacks from background threads
    std::function<void()> callback;
    while (mainThreadCallbacks.tryPop(callback)) {
        callback();
    }
}

void AppState::onTextMessageReceived(const Network::TextMessage& msg) {
    // Only add messages for the current channel
    if (msg.channel == currentChannel) {
        messages.push_back({
            msg.channel,
            msg.author,
            msg.content,
            msg.timestamp.empty() ? "" : msg.timestamp
        });
        
        // Update connection status
        connectionStatus = "Message received from " + msg.author + " on channel: " + msg.channel;
    } else {
        std::cout << "[MESSAGE] Received message for channel '" << msg.channel << "' but we're in '" << currentChannel << "' - ignoring" << std::endl;
    }
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
        // Save the server we connected to
        if (!serverAddress.empty() && !serverName.empty()) {
            addSavedServer(serverName);
            std::cout << "[CONNECTION] Saved server: " << serverName << std::endl;
        }
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
    
    // If this is a voice channel, also update voice channel users
    if (!currentUserListChannel.empty()) {
        // Check if this is a voice channel
        for (const auto& channel : channels) {
            if (channel.name == currentUserListChannel && !channel.isTextChannel) {
                setUsersInVoiceChannel(currentUserListChannel, users);
                std::cout << "[VOICE] Updated users in voice channel '" << currentUserListChannel << "': " << users.size() << " users" << std::endl;
                break;
            }
        }
    }
    
    // Also update friends list
    friends.clear();
    for (const auto& user : users) {
        friends.push_back({user, true}); // All users are online when received from server
    }
}

void AppState::onVoiceUserListReceived(const std::string& channel, const std::vector<std::string>& users) {
    setUsersInVoiceChannel(channel, users);
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
    
    // Set new current channel (text or voice)
    if (isTextChannel) {
        setCurrentChannel(channel);
        // Dołącz do kanału tekstowego na serwerze i pobierz listę użytkowników
        client->joinChannel(channel);
        currentUserListChannel = channel;
        client->requestUserList();
    } else {
        setCurrentVoiceChannel(channel);
        // Kanał głosowy — dołączenie obsługuje startVoice -> joinVoiceChannel
    }
    
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
    
    // Local echo - add channel immediately to UI
    channels.push_back({channelName, isTextChannel ? "#" : "🔊", isTextChannel});
    
    // If this is the server we're connected to, also request updated channel list
    if (client && client->isConnected()) {
        // Request updated channel list (will sync with server)
        client->requestChannelList();
    }
    
    connectionStatus = "Created channel: " + channelName;
    newChannelName.clear();
    
    // Switch to the new channel if it's a text channel
    if (isTextChannel) {
        switchChannel(channelName, true);
    }
}

// --- Voice Controls ---

void AppState::startVoice(const std::string& channel) {
    std::cout << "[VOICE] startVoice('" << channel << "') called" << std::endl;
    
    if (isVoiceActive) {
        std::cout << "[VOICE] Already active, skipping" << std::endl;
        return;
    }
    
    if (!client) {
        std::cerr << "[VOICE] ERROR: client is null!" << std::endl;
        return;
    }
    
    if (!client->isConnected()) {
        std::cerr << "[VOICE] ERROR: client not connected!" << std::endl;
        return;
    }
    
    std::cout << "[VOICE] Client is connected, proceeding..." << std::endl;
    
    // Initialize voice client if not already done (MUST be done in main thread)
    if (!voiceClient) {
        std::cout << "[VOICE] Creating VoiceClient instance..." << std::endl;
        voiceClient = std::make_unique<VoiceClient>();
        
        std::cout << "[VOICE] Initializing VoiceClient..." << std::endl;
        if (!voiceClient->initialize()) {
            std::cerr << "[VOICE] ERROR: Failed to initialize voice client!" << std::endl;
            connectionStatus = "Error: Failed to initialize voice client";
            voiceClient.reset();
            return;
        }
        std::cout << "[VOICE] VoiceClient initialized successfully" << std::endl;
    } else {
        std::cout << "[VOICE] VoiceClient already initialized, reusing" << std::endl;
    }
    
    // Dołącz do kanału głosowego na serwerze
    std::cout << "[VOICE] Joining voice channel: " << channel << std::endl;
    setCurrentVoiceChannel(channel);
    client->joinVoiceChannel(channel);
    
    // Start voice client with callback to send packets via network
    // Only send packets if there are other users in the channel
    std::cout << "[VOICE] Starting voice thread..." << std::endl;
    isVoiceActive = voiceClient->start(
        [this](const std::vector<uint8_t>& packet) {
            if (client && client->isConnected()) {
                client->sendVoicePacketUdp(packet);
            }
        },
        [this]() { return hasOtherUsersInVoiceChannel(); }
    );
    
    if (isVoiceActive) {
        std::cout << "[VOICE] SUCCESS: Voice active in channel: " << channel << std::endl;
        connectionStatus = "Voice active in channel: " + channel;
    } else {
        std::cerr << "[VOICE] ERROR: Failed to start voice thread!" << std::endl;
    }
}

void AppState::stopVoice() {
    std::cout << "[VOICE] stopVoice() called" << std::endl;
    
    if (!isVoiceActive) {
        std::cout << "[VOICE] Not active, skipping" << std::endl;
        return;
    }
    
    if (!voiceClient) {
        std::cerr << "[VOICE] ERROR: voiceClient is null!" << std::endl;
        return;
    }
    
    // Powiadom serwer że opuszczamy kanał głosowy
    if (client && client->isConnected()) {
        client->leaveVoiceChannel();
    }

    std::cout << "[VOICE] Stopping voice thread..." << std::endl;
    voiceClient->stop();
    isVoiceActive = false;
    setCurrentVoiceChannel("");
    std::cout << "[VOICE] Voice stopped successfully" << std::endl;
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
