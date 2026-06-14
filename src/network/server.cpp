#include "server.hpp"
#include <iostream>

namespace Network {

    Server::Server(uint16_t port, uint16_t voicePort)
        : port_(port), voicePort_(voicePort),
          ioContext_(),
          tcpAcceptor_(std::make_unique<tcp::acceptor>(ioContext_, tcp::endpoint(tcp::v4(), port))),
          udpSocket_(std::make_unique<udp::socket>(ioContext_, udp::endpoint(udp::v4(), voicePort))) {
        
        std::cout << "[SERVER CONSTRUCTOR] Server instance created for port " << port << " (TCP), " << voicePort << " (UDP)" << std::endl;
    }

    Server::~Server() {
        stop();
    }

    bool Server::start() {
        if (running_) {
            std::cerr << "Server is already running!" << std::endl;
            return false;
        }

        running_ = true;
        serverThread_ = std::jthread([this, stopToken = std::stop_token()]() {
            run(stopToken);
        });
        
        std::cout << "Server started on port " << port_ << " (TCP) and " << voicePort_ << " (UDP)" << std::endl;
        return true;
    }

    void Server::stop() {
        if (!running_) return;

        running_ = false;
        
        // Stop the io_context
        ioContext_.stop();
        
        // Close all client TCP sockets to unblock handleTcpClient threads
        std::lock_guard<std::mutex> lock(clientsMutex_);
        for (auto& pair : tcpClients_) {
            asio::error_code ec;
            pair.first->close(ec); // Close the socket to unblock read operations
        }
        tcpClients_.clear();
        clientsByUsername_.clear();
        
        // Close acceptor and UDP socket
        if (tcpAcceptor_) {
            tcpAcceptor_->close();
        }
        if (udpSocket_) {
            udpSocket_->close();
        }
        
        // Request stop for jthread and wait for it to finish
        if (serverThread_.joinable()) {
            serverThread_.request_stop();
            
            // Wait for thread to stop with timeout
            std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
            while (serverThread_.joinable() && 
                   std::chrono::steady_clock::now() - start < std::chrono::milliseconds(500)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            
            if (serverThread_.joinable()) {
                std::cerr << "[SERVER] Warning: Server thread did not stop within 500ms" << std::endl;
            }
        }
        
        std::cout << "Server stopped" << std::endl;
    }

    void Server::run(std::stop_token stopToken) {
        try {
            // Start accepting TCP connections
            acceptTcpConnections();
            
            // Start handling UDP packets
            handleUdpPackets();
            
            // Run the io_context until stopped
            // Use poll_one() instead of run_one() to avoid blocking
            while (!stopToken.stop_requested() && running_) {
                // poll_one() returns immediately if no handlers are ready
                // This allows the loop to check stopToken frequently
                if (ioContext_.poll_one() == 0) {
                    // No handlers were executed - sleep briefly to prevent CPU overload
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "Server error: " << e.what() << std::endl;
        }
    }

    void Server::acceptTcpConnections() {
        tcpAcceptor_->async_accept(
            [this](const asio::error_code& ec, tcp::socket socket) {
                if (!ec) {
                    std::cout << "New TCP connection from: " 
                              << socket.remote_endpoint() << std::endl;
                    
                    // Create a shared_ptr for the socket
                    auto socketPtr = std::make_shared<tcp::socket>(std::move(socket));
                    
                    // Handle the client in a new thread (for simplicity)
                    // In production, you might want to use asio::thread_pool
                    std::thread(&Server::handleTcpClient, this, socketPtr).detach();
                } else {
                    std::cerr << "TCP accept error: " << ec.message() << std::endl;
                }
                
                // Continue accepting new connections
                if (running_) {
                    acceptTcpConnections();
                }
            }
        );
    }

    void Server::handleTcpClient(std::shared_ptr<tcp::socket> socket) {
        try {
            // Create client info with the socket
            auto client = std::make_shared<ClientInfo>(std::move(*socket));
            
            // Read data from the client
            asio::streambuf buffer;
            
            while (running_) {
                asio::read_until(client->tcpSocket, buffer, '\n');
                
                std::istream is(&buffer);
                std::string messageStr;
                std::getline(is, messageStr);
                
                if (!messageStr.empty()) {
                    // Parse the message
                    Message msg = Message::deserialize(messageStr);
                    handleMessage(msg, client);
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "TCP client error: " << e.what() << std::endl;
        }
    }

    void Server::handleUdpPackets() {
        auto buffer = std::make_shared<std::vector<uint8_t>>(2048); // Max UDP packet size
        udp::endpoint senderEndpoint;
        
        udpSocket_->async_receive_from(
            asio::buffer(*buffer),
            senderEndpoint,
            [this, buffer, &senderEndpoint](const asio::error_code& ec, std::size_t bytesReceived) {
                if (!ec && bytesReceived > 0) {
                    // Resize the buffer to actual received size
                    buffer->resize(bytesReceived);
                    
                    // Check if we have at least 4 bytes (packet size header)
                    if (bytesReceived >= 4) {
                        uint32_t packetSize;
                        std::memcpy(&packetSize, buffer->data(), 4);
                        
                        if (bytesReceived >= 4 + packetSize) {
                            // Extract voice data
                            std::vector<uint8_t> voiceData(buffer->begin() + 4, buffer->begin() + 4 + packetSize);
                            
                            // Find the client by UDP endpoint and forward the packet
                            std::lock_guard<std::mutex> lock(clientsMutex_);
                            for (auto& pair : tcpClients_) {
                                auto client = pair.second;
                                if (client->udpEndpoint == senderEndpoint) {
                                    // Forward to all clients in the same channel
                                    VoicePacket pkt;
                                    pkt.channel = client->currentChannel;
                                    pkt.sender = client->username;
                                    pkt.audioData = voiceData;
                                    broadcastVoicePacket(pkt, client->currentChannel);
                                    break;
                                }
                            }
                        }
                    }
                } else if (ec != asio::error::operation_aborted) {
                    std::cerr << "UDP receive error: " << ec.message() << std::endl;
                }
                
                // Continue receiving
                if (running_) {
                    handleUdpPackets();
                }
            }
        );
    }

    void Server::handleMessage(const Message& msg, std::shared_ptr<ClientInfo> client) {
        switch (msg.type) {
            case MessageType::LOGIN_REQUEST: {
                LoginData loginData = LoginData::fromMessageData(msg.data);
                handleLoginRequest(loginData, client);
                break;
            }
            case MessageType::TEXT_MESSAGE: {
                TextMessage textMsg = TextMessage::fromMessageData(msg.data);
                handleTextMessage(textMsg, client);
                break;
            }
            case MessageType::VOICE_PACKET: {
                VoicePacket voicePkt = VoicePacket::fromMessageData(msg.data);
                handleVoicePacket(voicePkt, client);
                break;
            }
            case MessageType::JOIN_CHANNEL: {
                std::string channelName = msg.data.get("channel", "");
                handleJoinChannel(channelName, client);
                break;
            }
            case MessageType::CHANNEL_LIST_REQUEST: {
                // Send the channel list to the requesting client
                std::vector<ChannelInfo> channelList;
                {
                    std::lock_guard<std::mutex> lock(channelsMutex_);
                    for (const auto& pair : channels_) {
                        ChannelInfo info;
                        info.name = pair.second.name;
                        info.icon = pair.second.icon;
                        info.isTextChannel = pair.second.isTextChannel;
                        info.users.assign(pair.second.users.begin(), pair.second.users.end());
                        channelList.push_back(info);
                    }
                }
                
                // Build message data with channel list
                MessageData data;
                std::string channelsStr;
                for (const auto& channel : channelList) {
                    if (!channelsStr.empty()) channelsStr += ";";
                    channelsStr += channel.name + "," + channel.icon + "," + (channel.isTextChannel ? "true" : "false");
                }
                data.set("channels", channelsStr);
                
                sendMessageToClient(client, Message{
                    MessageType::CHANNEL_LIST_RESPONSE,
                    data
                });
                break;
            }
            case MessageType::USER_LIST_REQUEST: {
                std::string requestedChannel = msg.data.get("channel", client->currentChannel);
                
                // Get users in the requested channel
                std::vector<std::string> users;
                {
                    std::lock_guard<std::mutex> lock(channelsMutex_);
                    auto it = channels_.find(requestedChannel);
                    if (it != channels_.end()) {
                        users.assign(it->second.users.begin(), it->second.users.end());
                    }
                }
                
                // Build message data with user list
                MessageData data;
                std::string usersStr;
                for (const auto& user : users) {
                    if (!usersStr.empty()) usersStr += ",";
                    usersStr += user;
                }
                data.set("users", usersStr);
                data.set("channel", requestedChannel);
                
                sendMessageToClient(client, Message{
                    MessageType::USER_LIST_RESPONSE,
                    data
                });
                break;
            }
            case MessageType::DISCONNECT: {
                // Handle disconnect
                std::lock_guard<std::mutex> lock(clientsMutex_);
                tcpClients_.erase(&client->tcpSocket);
                clientsByUsername_.erase(client->username);
                
                // Remove from channel
                if (!client->currentChannel.empty()) {
                    std::lock_guard<std::mutex> channelLock(channelsMutex_);
                    auto it = channels_.find(client->currentChannel);
                    if (it != channels_.end()) {
                        it->second.users.erase(client->username);
                    }
                }
                break;
            }
            default: {
                std::cerr << "Unknown message type received" << std::endl;
                break;
            }
        }
    }

    void Server::handleTextMessage(const TextMessage& textMsg, std::shared_ptr<ClientInfo> client) {
        // Broadcast the message to all users in the same channel
        broadcastMessage(Message{
            MessageType::TEXT_MESSAGE,
            textMsg.toMessageData()
        }, textMsg.channel);
    }

    void Server::handleVoicePacket(const VoicePacket& voicePkt, std::shared_ptr<ClientInfo> client) {
        // Broadcast the voice packet to all users in the same channel
        broadcastVoicePacket(voicePkt, voicePkt.channel);
    }

    void Server::handleLoginRequest(const LoginData& loginData, std::shared_ptr<ClientInfo> client) {
        if (authenticateUser(loginData.username, loginData.password)) {
            // Login successful
            client->username = loginData.username;
            client->currentChannel = "ogólny"; // Default channel
            
            // Set UDP endpoint if port was provided
            if (loginData.udpPort > 0) {
                asio::ip::address tcpAddress = client->tcpSocket.remote_endpoint().address();
                client->udpEndpoint = udp::endpoint(tcpAddress, loginData.udpPort);
            }
            
            std::lock_guard<std::mutex> lock(clientsMutex_);
            tcpClients_[&client->tcpSocket] = client;
            clientsByUsername_[loginData.username] = client;
            
            // Add to default channel
            std::lock_guard<std::mutex> channelLock(channelsMutex_);
            auto it = channels_.find("ogólny");
            if (it != channels_.end()) {
                it->second.users.insert(loginData.username);
            }
            
            // Send success response
            LoginData response;
            response.username = loginData.username;
            response.success = true;
            sendMessageToClient(client, Message{
                MessageType::LOGIN_RESPONSE,
                response.toMessageData()
            });
            
            // Notify others that a new user joined
            MessageData userJoinedData;
            userJoinedData.set("username", loginData.username);
            broadcastMessage(Message{
                MessageType::USER_JOINED,
                userJoinedData
            });
        } else {
            // Login failed
            LoginData response;
            response.username = loginData.username;
            response.success = false;
            response.errorMessage = "Invalid username or password";
            sendMessageToClient(client, Message{
                MessageType::LOGIN_RESPONSE,
                response.toMessageData()
            });
        }
    }

    void Server::handleJoinChannel(const std::string& channelName, std::shared_ptr<ClientInfo> client) {
        std::lock_guard<std::mutex> lock(channelsMutex_);
        
        // Leave current channel
        if (!client->currentChannel.empty()) {
            auto it = channels_.find(client->currentChannel);
            if (it != channels_.end()) {
                it->second.users.erase(client->username);
            }
        }
        
        // Join new channel
        client->currentChannel = channelName;
        auto it = channels_.find(channelName);
        if (it != channels_.end()) {
            it->second.users.insert(client->username);
        }
        
        // Send channel list to client
        std::vector<ChannelInfo> channelList;
        for (const auto& pair : channels_) {
            ChannelInfo info;
            info.name = pair.second.name;
            info.icon = pair.second.icon;
            info.isTextChannel = pair.second.isTextChannel;
            info.users.assign(pair.second.users.begin(), pair.second.users.end());
            channelList.push_back(info);
        }
        
        // Build message data with channel list
        MessageData data;
        std::string channelsStr;
        for (const auto& channel : channelList) {
            if (!channelsStr.empty()) channelsStr += ";";
            channelsStr += channel.name + "," + channel.icon + "," + (channel.isTextChannel ? "true" : "false");
        }
        data.set("channels", channelsStr);
        
        sendMessageToClient(client, Message{
            MessageType::CHANNEL_LIST_RESPONSE,
            data
        });
    }

    void Server::broadcastMessage(const Message& msg, const std::string& channel) {
        std::lock_guard<std::mutex> lock(clientsMutex_);
        
        for (auto& pair : tcpClients_) {
            auto client = pair.second;
            
            // If channel is specified, only send to clients in that channel
            if (!channel.empty() && client->currentChannel != channel) {
                continue;
            }
            
            try {
                std::string msgStr = msg.serialize() + "\n";
                asio::write(client->tcpSocket, asio::buffer(msgStr));
            } catch (const std::exception& e) {
                std::cerr << "Error sending message to client: " << e.what() << std::endl;
            }
        }
    }

    void Server::broadcastChannelList() {
        std::cout << "[SERVER] broadcastChannelList() called" << std::endl;
        std::lock_guard<std::mutex> lock(clientsMutex_);
        
        // Build channel list string
        std::string channelsStr;
        {
            std::lock_guard<std::mutex> lock2(channelsMutex_);
            for (const auto& pair : channels_) {
                const auto& channel = pair.second;
                if (!channelsStr.empty()) {
                    channelsStr += ";";
                }
                channelsStr += channel.name + "," + channel.icon + "," + (channel.isTextChannel ? "true" : "false");
            }
        }
        
        std::cout << "[SERVER] Channel list string: " << channelsStr << std::endl;
        std::cout << "[SERVER] Number of connected clients: " << tcpClients_.size() << std::endl;
        
        // Send to all clients
        Message msg;
        msg.type = MessageType::CHANNEL_LIST_RESPONSE;
        msg.data.set("channels", channelsStr);
        
        for (auto& pair : tcpClients_) {
            try {
                std::string msgStr = msg.serialize() + "\n";
                asio::write(pair.second->tcpSocket, asio::buffer(msgStr));
                std::cout << "[SERVER] Channel list sent to a client" << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "[SERVER] ERROR sending channel list to client: " << e.what() << std::endl;
            }
        }
        std::cout << "[SERVER] broadcastChannelList() completed" << std::endl;
    }

    void Server::broadcastVoicePacket(const VoicePacket& pkt, const std::string& channel) {
        std::lock_guard<std::mutex> lock(clientsMutex_);
        
        for (auto& pair : tcpClients_) {
            auto client = pair.second;
            
            // Only send to clients in the same channel
            if (client->currentChannel != channel) {
                continue;
            }
            
            // Send via UDP for better performance
            try {
                // Format: first 4 bytes = packet size, then the actual data
                std::vector<uint8_t> packet;
                uint32_t size = static_cast<uint32_t>(pkt.audioData.size());
                packet.resize(4 + pkt.audioData.size());
                std::memcpy(packet.data(), &size, 4);
                std::memcpy(packet.data() + 4, pkt.audioData.data(), pkt.audioData.size());
                
                // Send to the client's UDP endpoint
                if (client->udpEndpoint.address().to_string() != "0.0.0.0") {
                    udpSocket_->send_to(asio::buffer(packet), client->udpEndpoint);
                }
            } catch (const std::exception& e) {
                std::cerr << "Error sending voice packet to client: " << e.what() << std::endl;
            }
        }
    }

    void Server::sendMessageToClient(std::shared_ptr<ClientInfo> client, const Message& msg) {
        try {
            std::string msgStr = msg.serialize() + "\n";
            asio::write(client->tcpSocket, asio::buffer(msgStr));
        } catch (const std::exception& e) {
            std::cerr << "Error sending message to client: " << e.what() << std::endl;
        }
    }

    bool Server::authenticateUser(const std::string& username, const std::string& password) {
        std::lock_guard<std::mutex> lock(usersMutex_);
        
        // For now, allow any non-empty username (no password required)
        // In a real implementation, you'd check the password
        if (username.empty()) {
            return false;
        }
        
        // If user doesn't exist, create them (for demo purposes)
        if (users_.find(username) == users_.end()) {
            addUser(username, password);
        }
        
        return true;
    }

    void Server::addUser(const std::string& username, const std::string& password) {
        std::lock_guard<std::mutex> lock(usersMutex_);
        users_[username] = {password};
    }

    void Server::addChannel(const std::string& name, const std::string& icon, bool isTextChannel) {
        std::cout << "[SERVER] Adding channel: " << name << " (" << (isTextChannel ? "text" : "voice") << ")" << std::endl;
        std::lock_guard<std::mutex> lock(channelsMutex_);
        channels_[name] = {name, icon, isTextChannel, {}};
        
        std::cout << "[SERVER] Channel added to server." << std::endl;
        // Note: Channel list broadcast is handled separately to avoid blocking the main thread
    }

    std::vector<ChannelInfo> Server::getChannels() const {
        std::lock_guard<std::mutex> lock(channelsMutex_);
        std::vector<ChannelInfo> result;
        
        for (const auto& pair : channels_) {
            ChannelInfo info;
            info.name = pair.second.name;
            info.icon = pair.second.icon;
            info.isTextChannel = pair.second.isTextChannel;
            info.users.assign(pair.second.users.begin(), pair.second.users.end());
            result.push_back(info);
        }
        
        return result;
    }

    std::vector<std::string> Server::getUsersInChannel(const std::string& channelName) const {
        std::lock_guard<std::mutex> lock(channelsMutex_);
        auto it = channels_.find(channelName);
        
        if (it != channels_.end()) {
            return std::vector<std::string>(it->second.users.begin(), it->second.users.end());
        }
        
        return {};
    }
}
