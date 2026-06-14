#ifndef ECHOHUB_NETWORK_PROTOCOL_HPP
#define ECHOHUB_NETWORK_PROTOCOL_HPP

#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

// Using nlohmann/json for easy JSON serialization
// If not available, we can use raw strings or implement a simple parser
using json = nlohmann::json;

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

    // Convert MessageType to string for JSON serialization
    inline std::string messageTypeToString(MessageType type) {
        switch (type) {
            case MessageType::CONNECT_REQUEST: return "connect_request";
            case MessageType::CONNECT_RESPONSE: return "connect_response";
            case MessageType::DISCONNECT: return "disconnect";
            case MessageType::LOGIN_REQUEST: return "login_request";
            case MessageType::LOGIN_RESPONSE: return "login_response";
            case MessageType::USER_JOINED: return "user_joined";
            case MessageType::USER_LEFT: return "user_left";
            case MessageType::TEXT_MESSAGE: return "text_message";
            case MessageType::VOICE_PACKET: return "voice_packet";
            case MessageType::JOIN_CHANNEL: return "join_channel";
            case MessageType::LEAVE_CHANNEL: return "leave_channel";
            case MessageType::CHANNEL_LIST_REQUEST: return "channel_list_request";
            case MessageType::CHANNEL_LIST_RESPONSE: return "channel_list_response";
            case MessageType::SERVER_INFO_REQUEST: return "server_info_request";
            case MessageType::SERVER_INFO_RESPONSE: return "server_info_response";
            case MessageType::ERROR: return "error";
            default: return "unknown";
        }
    }

    // Convert string to MessageType
    inline MessageType stringToMessageType(const std::string& str) {
        if (str == "connect_request") return MessageType::CONNECT_REQUEST;
        if (str == "connect_response") return MessageType::CONNECT_RESPONSE;
        if (str == "disconnect") return MessageType::DISCONNECT;
        if (str == "login_request") return MessageType::LOGIN_REQUEST;
        if (str == "login_response") return MessageType::LOGIN_RESPONSE;
        if (str == "user_joined") return MessageType::USER_JOINED;
        if (str == "user_left") return MessageType::USER_LEFT;
        if (str == "text_message") return MessageType::TEXT_MESSAGE;
        if (str == "voice_packet") return MessageType::VOICE_PACKET;
        if (str == "join_channel") return MessageType::JOIN_CHANNEL;
        if (str == "leave_channel") return MessageType::LEAVE_CHANNEL;
        if (str == "channel_list_request") return MessageType::CHANNEL_LIST_REQUEST;
        if (str == "channel_list_response") return MessageType::CHANNEL_LIST_RESPONSE;
        if (str == "server_info_request") return MessageType::SERVER_INFO_REQUEST;
        if (str == "server_info_response") return MessageType::SERVER_INFO_RESPONSE;
        if (str == "error") return MessageType::ERROR;
        return MessageType::ERROR;
    }

    // --- Message Structures ---
    
    /**
     * @brief Base structure for all network messages.
     * 
     * All messages have a type and can contain additional data.
     */
    struct Message {
        MessageType type;
        json data;
        
        // Serialize to JSON string
        std::string serialize() const {
            json j;
            j["type"] = messageTypeToString(type);
            j["data"] = data;
            return j.dump();
        }
        
        // Deserialize from JSON string
        static Message deserialize(const std::string& str) {
            try {
                json j = json::parse(str);
                Message msg;
                msg.type = stringToMessageType(j["type"]);
                msg.data = j["data"];
                return msg;
            } catch (...) {
                // Return error message on parse failure
                Message msg;
                msg.type = MessageType::ERROR;
                msg.data["error"] = "Invalid message format";
                return msg;
            }
        }
    };

    // --- Specific Message Types ---
    
    /**
     * @brief Text message for chat.
     */
    struct TextMessage {
        std::string channel;
        std::string author;
        std::string content;
        std::string timestamp;
        
        // Convert to JSON
        json toJson() const {
            json j;
            j["channel"] = channel;
            j["author"] = author;
            j["content"] = content;
            j["timestamp"] = timestamp;
            return j;
        }
        
        // Create from JSON
        static TextMessage fromJson(const json& j) {
            TextMessage msg;
            msg.channel = j.value("channel", "");
            msg.author = j.value("author", "");
            msg.content = j.value("content", "");
            msg.timestamp = j.value("timestamp", "");
            return msg;
        }
    };

    /**
     * @brief Voice packet for VoIP.
     */
    struct VoicePacket {
        std::string channel;
        std::string sender;
        std::vector<uint8_t> audioData; // Encoded Opus data
        
        // Convert to JSON (audio data as base64)
        json toJson() const {
            json j;
            j["channel"] = channel;
            j["sender"] = sender;
            // Encode binary data as base64 for JSON
            std::string base64Data = base64Encode(audioData);
            j["audio_data"] = base64Data;
            return j;
        }
        
        // Create from JSON
        static VoicePacket fromJson(const json& j) {
            VoicePacket pkt;
            pkt.channel = j.value("channel", "");
            pkt.sender = j.value("sender", "");
            std::string base64Data = j.value("audio_data", "");
            pkt.audioData = base64Decode(base64Data);
            return pkt;
        }
        
    private:
        // Simple base64 encoding/decoding (for JSON compatibility)
        static std::string base64Encode(const std::vector<uint8_t>& data);
        static std::vector<uint8_t> base64Decode(const std::string& data);
    };

    /**
     * @brief Login request/response.
     */
    struct LoginData {
        std::string username;
        std::string password;
        bool success = false;
        std::string errorMessage;
        
        json toJson() const {
            json j;
            j["username"] = username;
            j["password"] = password;
            j["success"] = success;
            j["error"] = errorMessage;
            return j;
        }
        
        static LoginData fromJson(const json& j) {
            LoginData login;
            login.username = j.value("username", "");
            login.password = j.value("password", "");
            login.success = j.value("success", false);
            login.errorMessage = j.value("error", "");
            return login;
        }
    };

    /**
     * @brief Channel information.
     */
    struct ChannelInfo {
        std::string name;
        std::string icon;
        bool isTextChannel;
        std::vector<std::string> users; // Users currently in this channel
        
        json toJson() const {
            json j;
            j["name"] = name;
            j["icon"] = icon;
            j["is_text_channel"] = isTextChannel;
            j["users"] = users;
            return j;
        }
        
        static ChannelInfo fromJson(const json& j) {
            ChannelInfo channel;
            channel.name = j.value("name", "");
            channel.icon = j.value("icon", "");
            channel.isTextChannel = j.value("is_text_channel", true);
            channel.users = j.value("users", std::vector<std::string>{});
            return channel;
        }
    };

    // --- Helper functions for base64 encoding ---
    inline std::string VoicePacket::base64Encode(const std::vector<uint8_t>& data) {
        static const std::string base64_chars = 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "0123456789+/";
        
        std::string ret;
        int i = 0;
        int j = 0;
        unsigned char char_array_3[3];
        unsigned char char_array_4[4];
        
        for (auto byte : data) {
            char_array_3[i++] = byte;
            if (i == 3) {
                char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                char_array_4[3] = char_array_3[2] & 0x3f;
                
                for (i = 0; i < 4; i++)
                    ret += base64_chars[char_array_4[i]];
                i = 0;
            }
        }
        
        if (i) {
            for (j = i; j < 3; j++)
                char_array_3[j] = '\0';
            
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;
            
            for (j = 0; j < i + 1; j++)
                ret += base64_chars[char_array_4[j]];
            
            while (i++ < 3)
                ret += '=';
        }
        
        return ret;
    }
    
    inline std::vector<uint8_t> VoicePacket::base64Decode(const std::string& data) {
        static const std::string base64_chars = 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "0123456789+/";
        
        std::vector<uint8_t> ret;
        int i = 0;
        int j = 0;
        int in_len = data.size();
        unsigned char char_array_4[4], char_array_3[3];
        
        while (in_len-- && (data[j] != '=')) {
            char_array_4[i++] = data[j];
            j++;
            if (i == 4) {
                for (i = 0; i < 4; i++)
                    char_array_4[i] = base64_chars.find(char_array_4[i]);
                
                char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
                char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
                char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
                
                for (i = 0; i < 3; i++)
                    ret.push_back(char_array_3[i]);
                i = 0;
            }
        }
        
        if (i) {
            for (int k = i; k < 4; k++)
                char_array_4[k] = 0;
            
            for (int k = 0; k < 4; k++)
                char_array_4[k] = base64_chars.find(char_array_4[k]);
            
            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
            
            for (int k = 0; k < i - 1; k++)
                ret.push_back(char_array_3[k]);
        }
        
        return ret;
    }
}

#endif // ECHOHUB_NETWORK_PROTOCOL_HPP
