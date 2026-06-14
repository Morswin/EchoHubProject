#ifndef ECHOHUB_NETWORK_SERVER_HPP
#define ECHOHUB_NETWORK_SERVER_HPP

#include <asio.hpp>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <thread>
#include <atomic>

#include "protocol.hpp"
#include "../utils/thread_safe_queue.hpp"

namespace Network {
    using asio::ip::tcp;
    using asio::ip::udp;

    /**
     * @brief Server for EchoHub application.
     * 
     * Handles TCP connections for text chat and UDP for voice packets.
     * Runs in a separate thread and manages client connections, channels, and users.
     */
    class Server {
    public:
        /**
         * @brief Constructor.
         * @param port TCP port for text chat.
         * @param voicePort UDP port for voice packets.
         */
        Server(uint16_t port = 9987, uint16_t voicePort = 9988);
        
        ~Server();

        /**
         * @brief Start the server in a separate thread.
         * @return true if server started successfully.
         */
        bool start();

        /**
         * @brief Stop the server.
         */
        void stop();

        /**
         * @brief Check if the server is running.
         */
        bool isRunning() const { return running_; }

        /**
         * @brief Get the TCP port.
         */
        uint16_t getPort() const { return port_; }

        /**
         * @brief Get the UDP port for voice.
         */
        uint16_t getVoicePort() const { return voicePort_; }

        /**
         * @brief Add a channel to the server.
         * @param name Channel name.
         * @param icon Channel icon.
         * @param isTextChannel Whether it's a text channel.
         */
        void addChannel(const std::string& name, const std::string& icon, bool isTextChannel);

        /**
         * @brief Get list of all channels.
         */
        std::vector<ChannelInfo> getChannels() const;

        /**
         * @brief Get users in a specific channel.
         */
        std::vector<std::string> getUsersInChannel(const std::string& channelName) const;

    private:
        // --- Server Configuration ---
        uint16_t port_;        // TCP port for text chat
        uint16_t voicePort_;   // UDP port for voice packets
        std::atomic<bool> running_{false};

        // --- Network Resources ---
        asio::io_context ioContext_;
        std::unique_ptr<tcp::acceptor> tcpAcceptor_;
        std::unique_ptr<udp::socket> udpSocket_;
        std::thread serverThread_;

        // --- Client Management ---
        struct ClientInfo {
            std::string username;
            std::string currentChannel;
            tcp::socket tcpSocket;
            udp::endpoint udpEndpoint;
            
            // Constructor that takes a socket (moved from another socket)
            explicit ClientInfo(tcp::socket socket) 
                : tcpSocket(std::move(socket)) {}
        };

        std::unordered_map<tcp::socket*, std::shared_ptr<ClientInfo>> tcpClients_;
        std::unordered_map<std::string, std::shared_ptr<ClientInfo>> clientsByUsername_;
        std::mutex clientsMutex_;

        // --- Channel Management ---
        struct Channel {
            std::string name;
            std::string icon;
            bool isTextChannel;
            std::unordered_set<std::string> users; // Usernames in this channel
        };

        std::unordered_map<std::string, Channel> channels_;
        mutable std::mutex channelsMutex_; // mutable to allow locking in const methods

        // --- User Management ---
        struct UserCredentials {
            std::string password;
        };

        std::unordered_map<std::string, UserCredentials> users_;
        std::mutex usersMutex_;

        // --- Message Queues ---
        ThreadSafeQueue<std::string> incomingMessages_; // For TCP messages
        ThreadSafeQueue<std::pair<udp::endpoint, std::vector<uint8_t>>> incomingVoicePackets_;

        // --- Server Operations ---
        void run(); // Main server loop
        void acceptTcpConnections();
        void handleTcpClient(std::shared_ptr<tcp::socket> socket);
        void handleUdpPackets();

        // --- Message Handlers ---
        void handleMessage(const Message& msg, std::shared_ptr<ClientInfo> client);
        void handleTextMessage(const TextMessage& textMsg, std::shared_ptr<ClientInfo> client);
        void handleVoicePacket(const VoicePacket& voicePkt, std::shared_ptr<ClientInfo> client);
        void handleLoginRequest(const LoginData& loginData, std::shared_ptr<ClientInfo> client);
        void handleJoinChannel(const std::string& channelName, std::shared_ptr<ClientInfo> client);

        // --- Helper Methods ---
        void broadcastMessage(const Message& msg, const std::string& channel = "");
        void broadcastVoicePacket(const VoicePacket& pkt, const std::string& channel);
        void sendMessageToClient(std::shared_ptr<ClientInfo> client, const Message& msg);
        bool authenticateUser(const std::string& username, const std::string& password);
        void addUser(const std::string& username, const std::string& password);
    };
}

#endif // ECHOHUB_NETWORK_SERVER_HPP
