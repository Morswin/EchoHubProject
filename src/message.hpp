#ifndef ECHOHUBPROJECT_MESSAGE_HPP
#define ECHOHUBPROJECT_MESSAGE_HPP

#include <chrono>
#include <string>
#include <format>


using datatime = std::chrono::system_clock::time_point;

class Message {
private:
    inline static int s_NextID = 0;
    int m_ID;
    datatime m_DataTimeSent;
    std::string m_Content;
    std::string m_Author;
public:
    Message(std::string m_Content, std::string m_Author);
    [[nodiscard]] std::string GetDataTime() const;
    [[nodiscard]] const std::string& GetAuthor() const;
    [[nodiscard]] const std::string& GetContent() const;
    [[nodiscard]] int GetID() const;
};


#endif //ECHOHUBPROJECT_MESSAGE_HPP