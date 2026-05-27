#include "message.hpp"
#include <chrono>
#include <string>
#include <format>

Message::Message(std::string m_Content, std::string m_Author) : m_Content(std::move(m_Content)), m_Author(std::move(m_Author)) {
    this->m_DataTimeSent = std::chrono::system_clock::now();
    this->m_ID = s_NextID++;
}

std::string Message::GetDataTime() const {
    return std::format("{:%Y-%m-%d %H:%M:%S}", this->m_DataTimeSent);
}

const std::string& Message::GetAuthor() const {
    return this->m_Author;
}

const std::string& Message::GetContent() const {
    return this->m_Content;
}

int Message::GetID() const {
    return this->m_ID;
}
