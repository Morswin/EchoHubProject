#ifndef ECHOHUB_NETWORK_PROTOCOL_HPP
#define ECHOHUB_NETWORK_PROTOCOL_HPP

#include <string>
#include <vector>
#include <cstdint>
#include <map>
#include <sstream>

namespace Network {
    // --- Message Types ---
    enum class MessageType {
        // Connection management
        CONNECT_REQUEST,
        CONNECT_RESPONSE,
        DISCONNECT,
        
        // User management
        LOGIN_REQUEST,
        LOGIN_RESPONSE,
        USER_JOINED,
        USER_LEFT,
        
        // Text chat
        TEXT_MESSAGE,
        
        // Voice chat
        VOICE_PACKET,
        
        // Channel management
        JOIN_CHANNEL,
        LEAVE_CHANNEL,
        CHANNEL_LIST_REQUEST,
        CHANNEL_LIST_RESPONSE,
        
        // Server info
        SERVER_INFO_REQUEST,
        SERVER_INFO_RESPONSE,
        
        // Error handling
        ERROR
    };

    // Convert MessageType to string
    inline std::string messageTypeToString(MessageType type) {
        switch (type) {
            case MessageType::CONNECT_REQUEST: return "CONNECT_REQUEST";
            case MessageType::CONNECT_RESPONSE: return "CONNECT_RESPONSE";
            case MessageType::DISCONNECT: return "DISCONNECT";
            case MessageType::LOGIN_REQUEST: return "LOGIN_REQUEST";
            case MessageType::LOGIN_RESPONSE: return "LOGIN_RESPONSE";
            case MessageType::USER_JOINED: return "USER_JOINED";
            case MessageType::USER_LEFT: return "USER_LEFT";
            case MessageType::TEXT_MESSAGE: return "TEXT_MESSAGE";
            case MessageType::VOICE_PACKET: return "VOICE_PACKET";
            case MessageType::JOIN_CHANNEL: return "JOIN_CHANNEL";
            case MessageType::LEAVE_CHANNEL: return "LEAVE_CHANNEL";
            case MessageType::CHANNEL_LIST_REQUEST: return "CHANNEL_LIST_REQUEST";
            case MessageType::CHANNEL_LIST_RESPONSE: return "CHANNEL_LIST_RESPONSE";
            case MessageType::SERVER_INFO_REQUEST: return "SERVER_INFO_REQUEST";
            case MessageType::SERVER_INFO_RESPONSE: return "SERVER_INFO_RESPONSE";
            case MessageType::ERROR: return "ERROR";
            default: return "UNKNOWN";
        }
    }

    // Convert string to MessageType
    inline MessageType stringToMessageType(const std::string& str) {
        if (str == "CONNECT_REQUEST") return MessageType::CONNECT_REQUEST;
        if (str == "CONNECT_RESPONSE") return MessageType::CONNECT_RESPONSE;
        if (str == "DISCONNECT") return MessageType::DISCONNECT;
        if (str == "LOGIN_REQUEST") return MessageType::LOGIN_REQUEST;
        if (str == "LOGIN_RESPONSE") return MessageType::LOGIN_RESPONSE;
        if (str == "USER_JOINED") return MessageType::USER_JOINED;
        if (str == "USER_LEFT") return MessageType::USER_LEFT;
        if (str == "TEXT_MESSAGE") return MessageType::TEXT_MESSAGE;
        if (str == "VOICE_PACKET") return MessageType::VOICE_PACKET;
        if (str == "JOIN_CHANNEL") return MessageType::JOIN_CHANNEL;
        if (str == "LEAVE_CHANNEL") return MessageType::LEAVE_CHANNEL;
        if (str == "CHANNEL_LIST_REQUEST") return MessageType::CHANNEL_LIST_REQUEST;
        if (str == "CHANNEL_LIST_RESPONSE") return MessageType::CHANNEL_LIST_RESPONSE;
        if (str == "SERVER_INFO_REQUEST") return MessageType::SERVER_INFO_REQUEST;
        if (str == "SERVER_INFO_RESPONSE") return MessageType::SERVER_INFO_RESPONSE;
        if (str == "ERROR") return MessageType::ERROR;
        return MessageType::ERROR;
    }

    // --- Simple Key-Value Parser ---
    // Format: "KEY1:value1|KEY2:value2|KEY3:value3"
    class MessageData {
    private:
        std::map<std::string, std::string> data_;

    public:
        MessageData() = default;
        
        // Parse from string
        void parse(const std::string& str) {
            data_.clear();
            size_t start = 0;
            size_t end = str.find('|');
            
            while (end != std::string::npos) {
                std::string pair = str.substr(start, end - start);
                size_t colon = pair.find(':');
                if (colon != std::string::npos) {
                    std::string key = pair.substr(0, colon);
                    std::string value = pair.substr(colon + 1);
                    data_[key] = value;
                }
                start = end + 1;
                end = str.find('|', start);
            }
            
            // Last pair
            if (start < str.size()) {
                std::string pair = str.substr(start);
                size_t colon = pair.find(':');
                if (colon != std::string::npos) {
                    std::string key = pair.substr(0, colon);
                    std::string value = pair.substr(colon + 1);
                    data_[key] = value;
                }
            }
        }
        
        // Serialize to string
        std::string serialize() const {
            std::ostringstream oss;
            bool first = true;
            for (const auto& pair : data_) {
                if (!first) oss << "|";
                oss << pair.first << ":" << pair.second;
                first = false;
            }
            return oss.str();
        }
        
        // Accessors
        std::string get(const std::string& key, const std::string& defaultValue = "") const {
            auto it = data_.find(key);
            if (it != data_.end()) return it->second;
            return defaultValue;
        }
        
        void set(const std::string& key, const std::string& value) {
            data_[key] = value;
        }
        
        bool contains(const std::string& key) const {
            return data_.find(key) != data_.end();
        }
        
        const std::map<std::string, std::string>& getAll() const {
            return data_;
        }
    };

    // --- Message Structure ---
    struct Message {
        MessageType type;
        MessageData data;
        
        // Serialize to string: "TYPE:type_string|..."
        std::string serialize() const {
            std::ostringstream oss;
            oss << "TYPE:" << messageTypeToString(type);
            std::string dataStr = data.serialize();
            if (!dataStr.empty()) {
                oss << "|" << dataStr;
            }
            return oss.str();
        }
        
        // Deserialize from string
        static Message deserialize(const std::string& str) {
            Message msg;
            size_t typeEnd = str.find('|');
            std::string typeStr;
            std::string dataStr;
            
            if (typeEnd != std::string::npos) {
                typeStr = str.substr(5, typeEnd - 5); // Skip "TYPE:"
                dataStr = str.substr(typeEnd + 1);
            } else {
                // Only type, no data
                if (str.find("TYPE:") == 0) {
                    typeStr = str.substr(5);
                } else {
                    typeStr = str;
                }
            }
            
            msg.type = stringToMessageType(typeStr);
            if (!dataStr.empty()) {
                msg.data.parse(dataStr);
            }
            return msg;
        }
    };

    // --- Specific Message Types ---
    
    struct TextMessage {
        std::string channel;
        std::string author;
        std::string content;
        std::string timestamp;
        
        // Convert to MessageData
        MessageData toMessageData() const {
            MessageData data;
            data.set("channel", channel);
            data.set("author", author);
            data.set("content", content);
            data.set("timestamp", timestamp);
            return data;
        }
        
        // Create from MessageData
        static TextMessage fromMessageData(const MessageData& data) {
            TextMessage msg;
            msg.channel = data.get("channel", "");
            msg.author = data.get("author", "");
            msg.content = data.get("content", "");
            msg.timestamp = data.get("timestamp", "");
            return msg;
        }
    };

    struct VoicePacket {
        std::string channel;
        std::string sender;
        std::vector<uint8_t> audioData; // Encoded Opus data
        
        // Convert to MessageData (audio data as hex string for simplicity)
        MessageData toMessageData() const {
            MessageData data;
            data.set("channel", channel);
            data.set("sender", sender);
            // Convert binary data to hex string
            std::string hexData;
            for (uint8_t byte : audioData) {
                char buf[3];
                snprintf(buf, sizeof(buf), "%02x", byte);
                hexData += buf;
            }
            data.set("audio_data", hexData);
            return data;
        }
        
        // Create from MessageData
        static VoicePacket fromMessageData(const MessageData& data) {
            VoicePacket pkt;
            pkt.channel = data.get("channel", "");
            pkt.sender = data.get("sender", "");
            std::string hexData = data.get("audio_data", "");
            // Convert hex string back to binary
            for (size_t i = 0; i < hexData.size(); i += 2) {
                std::string byteString = hexData.substr(i, 2);
                uint8_t byte = static_cast<uint8_t>(std::stoi(byteString, nullptr, 16));
                pkt.audioData.push_back(byte);
            }
            return pkt;
        }
    };

    struct LoginData {
        std::string username;
        std::string password;
        bool success = false;
        std::string errorMessage;
        
        // Convert to MessageData
        MessageData toMessageData() const {
            MessageData data;
            data.set("username", username);
            data.set("password", password);
            data.set("success", success ? "true" : "false");
            data.set("error", errorMessage);
            return data;
        }
        
        // Create from MessageData
        static LoginData fromMessageData(const MessageData& data) {
            LoginData login;
            login.username = data.get("username", "");
            login.password = data.get("password", "");
            login.success = (data.get("success", "") == "true");
            login.errorMessage = data.get("error", "");
            return login;
        }
    };

    struct ChannelInfo {
        std::string name;
        std::string icon;
        bool isTextChannel;
        std::vector<std::string> users;
        
        // Convert to MessageData (users as comma-separated string)
        MessageData toMessageData() const {
            MessageData data;
            data.set("name", name);
            data.set("icon", icon);
            data.set("is_text_channel", isTextChannel ? "true" : "false");
            // Convert users to comma-separated string
            std::string usersStr;
            for (size_t i = 0; i < users.size(); ++i) {
                if (i > 0) usersStr += ",";
                usersStr += users[i];
            }
            data.set("users", usersStr);
            return data;
        }
        
        // Create from MessageData
        static ChannelInfo fromMessageData(const MessageData& data) {
            ChannelInfo channel;
            channel.name = data.get("name", "");
            channel.icon = data.get("icon", "");
            channel.isTextChannel = (data.get("is_text_channel", "") == "true");
            // Parse users from comma-separated string
            std::string usersStr = data.get("users", "");
            size_t start = 0;
            size_t end = usersStr.find(',');
            while (end != std::string::npos) {
                channel.users.push_back(usersStr.substr(start, end - start));
                start = end + 1;
                end = usersStr.find(',', start);
            }
            if (start < usersStr.size()) {
                channel.users.push_back(usersStr.substr(start));
            }
            return channel;
        }
    };
}

#endif // ECHOHUB_NETWORK_PROTOCOL_HPP
