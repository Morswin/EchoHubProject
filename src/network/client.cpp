#include "client.hpp"
#include <iostream>

namespace Network {

    Client::Client(const std::string& serverAddress, uint16_t serverPort, uint16_t voicePort)
        : serverAddress_(serverAddress),
          serverPort_(serverPort),
          voicePort_(voicePort),
          ioContext_(),
          tcpSocket_(std::make_unique<tcp::socket>(ioContext_)),
          udpSocket_(std::make_unique<udp::socket>(ioContext_)) {
    }

    Client::~Client() {
        disconnect();
    }

    bool Client::connect(const std::string& username, const std::string& password) {
        if (connected_) {
            std::cerr << "Already connected to server!" << std::endl;
            return false;
        }

        username_ = username;
        
        try {
            // Resolve server address
            tcp::resolver resolver(ioContext_);
            auto endpoints = resolver.resolve(serverAddress_, std::to_string(serverPort_));
            
            // Connect to server
            asio::connect(*tcpSocket_, endpoints);
            
            // Send login request
            LoginData loginData;
            loginData.username = username;
            loginData.password = password;
            
            Message loginMsg;
            loginMsg.type = MessageType::LOGIN_REQUEST;
            loginMsg.data = loginData.toJson();
            
            sendMessage(loginMsg);
            
            // Start the client thread
            connected_ = true;
            clientThread_ = std::thread(&Client::run, this);
            
            if (connectionCallback_) {
                connectionCallback_(true, "Connected to server");
            }
            
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Connection error: " << e.what() << std::endl;
            if (connectionCallback_) {
                connectionCallback_(false, e.what());
            }
            return false;
        }
    }

    void Client::disconnect() {
        if (!connected_) return;

        connected_ = false;
        
        // Send disconnect message
        try {
            Message disconnectMsg;
            disconnectMsg.type = MessageType::DISCONNECT;
            sendMessage(disconnectMsg);
        } catch (...) {
            // Ignore errors during disconnect
        }
        
        // Stop the io_context
        ioContext_.stop();
        
        // Close sockets
        if (tcpSocket_) {
            tcpSocket_->close();
        }
        if (udpSocket_) {
            udpSocket_->close();
        }
        
        // Join the client thread
        if (clientThread_.joinable()) {
            clientThread_.join();
        }
        
        if (connectionCallback_) {
            connectionCallback_(false, "Disconnected from server");
        }
    }

    void Client::run() {
        try {
            // Start reading messages
            readMessages();
            
            // Run the io_context
            ioContext_.run();
        } catch (const std::exception& e) {
            std::cerr << "Client error: " << e.what() << std::endl;
            if (connectionCallback_) {
                connectionCallback_(false, e.what());
            }
        }
    }

    void Client::readMessages() {
        auto buffer = std::make_shared<asio::streambuf>();
        
        asio::async_read_until(
            *tcpSocket_,
            *buffer,
            '\n',
            [this, buffer](const asio::error_code& ec, std::size_t bytesRead) {
                if (!ec && bytesRead > 0) {
                    std::istream is(buffer.get());
                    std::string messageStr;
                    std::getline(is, messageStr);
                    
                    if (!messageStr.empty()) {
                        // Parse the message
                        Message msg = Message::deserialize(messageStr);
                        handleMessage(msg);
                    }
                    
                    // Continue reading
                    if (connected_) {
                        readMessages();
                    }
                } else if (ec != asio::error::operation_aborted) {
                    std::cerr << "Read error: " << ec.message() << std::endl;
                    if (connectionCallback_) {
                        connectionCallback_(false, "Connection lost: " + ec.message());
                    }
                }
            }
        );
    }

    void Client::handleMessage(const Message& msg) {
        switch (msg.type) {
            case MessageType::LOGIN_RESPONSE: {
                LoginData loginData = LoginData::fromJson(msg.data);
                handleLoginResponse(loginData);
                break;
            }
            case MessageType::TEXT_MESSAGE: {
                TextMessage textMsg = TextMessage::fromJson(msg.data);
                handleTextMessage(textMsg);
                break;
            }
            case MessageType::VOICE_PACKET: {
                VoicePacket voicePkt = VoicePacket::fromJson(msg.data);
                handleVoicePacket(voicePkt);
                break;
            }
            case MessageType::CHANNEL_LIST_RESPONSE: {
                handleChannelListResponse(msg.data);
                break;
            }
            case MessageType::USER_JOINED: {
                handleUserJoined(msg.data);
                break;
            }
            case MessageType::USER_LEFT: {
                handleUserLeft(msg.data);
                break;
            }
            case MessageType::ERROR: {
                std::cerr << "Server error: " << msg.data.value("error", "Unknown error") << std::endl;
                break;
            }
            default: {
                std::cerr << "Unknown message type received" << std::endl;
                break;
            }
        }
    }

    void Client::handleTextMessage(const TextMessage& textMsg) {
        if (textMessageCallback_) {
            textMessageCallback_(textMsg);
        }
    }

    void Client::handleVoicePacket(const VoicePacket& voicePkt) {
        if (voicePacketCallback_) {
            voicePacketCallback_(voicePkt);
        }
    }

    void Client::handleLoginResponse(const LoginData& loginData) {
        if (!loginData.success) {
            std::cerr << "Login failed: " << loginData.errorMessage << std::endl;
            if (connectionCallback_) {
                connectionCallback_(false, "Login failed: " + loginData.errorMessage);
            }
            disconnect();
        } else {
            std::cout << "Login successful as: " << loginData.username << std::endl;
            
            // Request channel list after successful login
            requestChannelList();
        }
    }

    void Client::handleChannelListResponse(const json& data) {
        if (channelListCallback_) {
            std::vector<ChannelInfo> channels;
            if (data.contains("channels") && data["channels"].is_array()) {
                for (const auto& channelJson : data["channels"]) {
                    channels.push_back(ChannelInfo::fromJson(channelJson));
                }
            }
            channelListCallback_(channels);
        }
    }

    void Client::handleUserJoined(const json& data) {
        if (userListCallback_) {
            std::vector<std::string> users;
            if (data.contains("username")) {
                users.push_back(data["username"]);
            }
            // In a real implementation, you'd maintain a full user list
            userListCallback_(users);
        }
    }

    void Client::handleUserLeft(const json& data) {
        if (userListCallback_) {
            std::vector<std::string> users;
            // In a real implementation, you'd remove the user from the list
            userListCallback_(users);
        }
    }

    void Client::sendTextMessage(const std::string& channel, const std::string& content) {
        if (!connected_) return;

        TextMessage textMsg;
        textMsg.channel = channel;
        textMsg.author = username_;
        textMsg.content = content;
        textMsg.timestamp = ""; // Timestamp can be set by the server or client

        Message msg;
        msg.type = MessageType::TEXT_MESSAGE;
        msg.data = textMsg.toJson();

        sendMessage(msg);
    }

    void Client::sendVoicePacket(const std::string& channel, const std::vector<uint8_t>& audioData) {
        if (!connected_) return;

        VoicePacket voicePkt;
        voicePkt.channel = channel;
        voicePkt.sender = username_;
        voicePkt.audioData = audioData;

        Message msg;
        msg.type = MessageType::VOICE_PACKET;
        msg.data = voicePkt.toJson();

        sendMessage(msg);
    }

    void Client::joinChannel(const std::string& channelName) {
        if (!connected_) return;

        currentChannel_ = channelName;

        Message msg;
        msg.type = MessageType::JOIN_CHANNEL;
        msg.data = {{{"channel", channelName}}};

        sendMessage(msg);
    }

    void Client::leaveChannel() {
        if (!connected_) return;

        Message msg;
        msg.type = MessageType::LEAVE_CHANNEL;
        msg.data = {{{"channel", currentChannel_}}};

        sendMessage(msg);
        currentChannel_ = "";
    }

    void Client::requestChannelList() {
        if (!connected_) return;

        Message msg;
        msg.type = MessageType::CHANNEL_LIST_REQUEST;
        msg.data = {};

        sendMessage(msg);
    }

    void Client::sendMessage(const Message& msg) {
        try {
            std::string msgStr = msg.serialize() + "\n";
            asio::write(*tcpSocket_, asio::buffer(msgStr));
        } catch (const std::exception& e) {
            std::cerr << "Error sending message: " << e.what() << std::endl;
        }
    }

    void Client::sendUdpPacket(const std::vector<uint8_t>& data, const udp::endpoint& endpoint) {
        try {
            udpSocket_->send_to(asio::buffer(data), endpoint);
        } catch (const std::exception& e) {
            std::cerr << "Error sending UDP packet: " << e.what() << std::endl;
        }
    }
}
