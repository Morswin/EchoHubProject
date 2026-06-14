#include "server.hpp"
#include <iostream>

namespace Network {

    Server::Server(uint16_t port, uint16_t voicePort)
        : port_(port), voicePort_(voicePort),
          ioContext_(),
          tcpAcceptor_(std::make_unique<tcp::acceptor>(ioContext_, tcp::endpoint(tcp::v4(), port))),
          udpSocket_(std::make_unique<udp::socket>(ioContext_, udp::endpoint(udp::v4(), voicePort))) {
        
        // Add default channels
        addChannel("ogólny", "#", true);
        addChannel("pomoc-kod", "#", true);
        addChannel("Poczekalnia", "🔊", false);
        addChannel("Pokój gier", "🔊", false);
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
        serverThread_ = std::thread(&Server::run, this);
        
        std::cout << "Server started on port " << port_ << " (TCP) and " << voicePort_ << " (UDP)" << std::endl;
        return true;
    }

    void Server::stop() {
        if (!running_) return;

        running_ = false;
        
        // Stop the io_context
        ioContext_.stop();
        
        // Close sockets
        if (tcpAcceptor_) {
            tcpAcceptor_->close();
        }
        if (udpSocket_) {
            udpSocket_->close();
        }
        
        // Join the server thread
        if (serverThread_.joinable()) {
            serverThread_.join();
        }
        
        std::cout << "Server stopped" << std::endl;
    }

    void Server::run() {
        try {
            // Start accepting TCP connections
            acceptTcpConnections();
            
            // Start handling UDP packets
            handleUdpPackets();
            
            // Run the io_context
            ioContext_.run();
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
            // Create client info
            auto client = std::make_shared<ClientInfo>();
            client->tcpSocket = std::move(*socket);
            
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
        auto buffer = std::make_shared<std::vector<uint8_t>>(1500); // Max UDP packet size
        
        udpSocket_->async_receive_from(
            asio::buffer(*buffer),
            [this, buffer](const asio::error_code& ec, std::size_t bytesReceived, udp::endpoint endpoint) {
                if (!ec && bytesReceived > 0) {
                    // Resize the buffer to actual received size
                    buffer->resize(bytesReceived);
                    
                    // For now, just store the packet
                    // In a real implementation, we'd parse it and forward to the appropriate channel
                    incomingVoicePackets_.push({endpoint, *buffer});
                } else if (ec) {
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

    void Server::broadcastVoicePacket(const VoicePacket& pkt, const std::string& channel) {
        std::lock_guard<std::mutex> lock(clientsMutex_);
        
        for (auto& pair : tcpClients_) {
            auto client = pair.second;
            
            // Only send to clients in the same channel
            if (client->currentChannel != channel) {
                continue;
            }
            
            // In a real implementation, we'd send the voice packet via UDP
            // For now, we'll just broadcast it via TCP (for simplicity)
            Message msg;
            msg.type = MessageType::VOICE_PACKET;
            msg.data = pkt.toMessageData();
            
            try {
                std::string msgStr = msg.serialize() + "\n";
                asio::write(client->tcpSocket, asio::buffer(msgStr));
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
        std::lock_guard<std::mutex> lock(channelsMutex_);
        channels_[name] = {name, icon, isTextChannel, {}};
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
