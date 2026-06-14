#ifndef ECHOHUB_NETWORK_CLIENT_HPP
#define ECHOHUB_NETWORK_CLIENT_HPP

#include <asio.hpp>
#include <memory>
#include <vector>
#include <string>
#include <thread>
#include <atomic>
#include <functional>

#include "protocol.hpp"
#include "../utils/thread_safe_queue.hpp"

namespace Network {
    using asio::ip::tcp;
    using asio::ip::udp;

    /**
     * @brief Client for EchoHub application.
     * 
     * Connects to a server for text chat (TCP) and voice (UDP).
     * Runs in a separate thread and provides callbacks for incoming messages.
     */
    class Client {
    public:
        /**
         * @brief Callback for incoming text messages.
         */
        using TextMessageCallback = std::function<void(const TextMessage&)>;
        
        /**
         * @brief Callback for incoming voice packets.
         */
        using VoicePacketCallback = std::function<void(const VoicePacket&)>;
        
        /**
         * @brief Callback for connection status changes.
         */
        using ConnectionCallback = std::function<void(bool connected, const std::string& message)>;
        
        /**
         * @brief Callback for channel list updates.
         */
        using ChannelListCallback = std::function<void(const std::vector<ChannelInfo>&)>;
        
        /**
         * @brief Callback for user list updates.
         */
        using UserListCallback = std::function<void(const std::vector<std::string>&)>;

        /**
         * @brief Constructor.
         * @param serverAddress Server IP address or hostname.
         * @param serverPort Server TCP port.
         * @param voicePort Server UDP port for voice.
         */
        Client(const std::string& serverAddress = "127.0.0.1", 
               uint16_t serverPort = 9987, 
               uint16_t voicePort = 9988);
        
        ~Client();

        /**
         * @brief Connect to the server.
         * @param username User's username.
         * @param password User's password.
         * @return true if connection succeeded.
         */
        bool connect(const std::string& username, const std::string& password);

        /**
         * @brief Disconnect from the server.
         */
        void disconnect();

        /**
         * @brief Check if connected to the server.
         */
        bool isConnected() const { return connected_; }

        /**
         * @brief Send a text message.
         * @param channel Channel to send the message to.
         * @param content Message content.
         */
        void sendTextMessage(const std::string& channel, const std::string& content);

        /**
         * @brief Send a voice packet.
         * @param channel Channel to send the voice packet to.
         * @param audioData Encoded Opus audio data.
         */
        void sendVoicePacket(const std::string& channel, const std::vector<uint8_t>& audioData);

        /**
         * @brief Send a raw voice packet via UDP (for use with VoiceClient callback).
         * @param audioData Encoded Opus audio data.
         */
        void sendVoicePacketUdp(const std::vector<uint8_t>& audioData);

        /**
         * @brief Join a channel.
         * @param channelName Name of the channel to join.
         */
        void joinChannel(const std::string& channelName);

        /**
         * @brief Leave the current channel.
         */
        void leaveChannel();

        /**
         * @brief Set the current voice channel for UDP packets.
         */
        void setVoiceChannel(const std::string& channel);

        /**
         * @brief Get the server's UDP endpoint for voice packets.
         */
        udp::endpoint getVoiceEndpoint() const { return voiceEndpoint_; }

        /**
         * @brief Set callback for incoming text messages.
         */
        void setTextMessageCallback(TextMessageCallback callback) {
            textMessageCallback_ = callback;
        }

        /**
         * @brief Set callback for incoming voice packets.
         */
        void setVoicePacketCallback(VoicePacketCallback callback) {
            voicePacketCallback_ = callback;
        }

        /**
         * @brief Set callback for connection status changes.
         */
        void setConnectionCallback(ConnectionCallback callback) {
            connectionCallback_ = callback;
        }

        /**
         * @brief Set callback for channel list updates.
         */
        void setChannelListCallback(ChannelListCallback callback) {
            channelListCallback_ = callback;
        }

        /**
         * @brief Set callback for user list updates.
         */
        void setUserListCallback(UserListCallback callback) {
            userListCallback_ = callback;
        }

        /**
         * @brief Request the list of channels from the server.
         */
        void requestChannelList();
        
        /**
         * @brief Request the list of users in the current channel.
         */
        void requestUserList();

        /**
         * @brief Get the current channel.
         */
        std::string getCurrentChannel() const { return currentChannel_; }

        /**
         * @brief Get the username.
         */
        std::string getUsername() const { return username_; }

    private:
        // --- Client Configuration ---
        std::string serverAddress_;
        uint16_t serverPort_;
        uint16_t voicePort_;
        std::string username_;
        std::string currentChannel_ = "ogólny"; // Default channel
        std::string voiceChannel_ = "ogólny"; // Default voice channel
        std::atomic<bool> connected_{false};

        // --- Network Resources ---
        asio::io_context ioContext_;
        std::unique_ptr<tcp::socket> tcpSocket_;
        std::unique_ptr<udp::socket> udpSocket_;
        udp::endpoint voiceEndpoint_;
        std::thread clientThread_;
        std::thread udpThread_;

        // --- Callbacks ---
        TextMessageCallback textMessageCallback_;
        VoicePacketCallback voicePacketCallback_;
        ConnectionCallback connectionCallback_;
        ChannelListCallback channelListCallback_;
        UserListCallback userListCallback_;

        // --- Message Queues ---
        ThreadSafeQueue<std::string> outgoingMessages_;

        // --- Client Operations ---
        void run(); // Main client loop
        void readMessages();
        void sendQueuedMessages();

        // --- Message Handlers ---
        void handleMessage(const Message& msg);
        void handleTextMessage(const TextMessage& textMsg);
        void handleVoicePacket(const VoicePacket& voicePkt);
        void handleLoginResponse(const LoginData& loginData);
        void handleChannelListResponse(const MessageData& data);
        void handleUserListResponse(const MessageData& data);
        void handleUserJoined(const MessageData& data);
        void handleUserLeft(const MessageData& data);

        // --- Helper Methods ---
        void sendMessage(const Message& msg);
        void sendUdpPacket(const std::vector<uint8_t>& data, const udp::endpoint& endpoint);
        
        // --- UDP Voice Methods ---
        void startUdpListener();
        void handleUdpPacket(const std::vector<uint8_t>& data, const udp::endpoint& endpoint);
        void udpListenLoop();
    };
}

#endif // ECHOHUB_NETWORK_CLIENT_HPP
