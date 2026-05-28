#ifndef ECHOHUBPROJECT_MOLECULES_HPP
#define ECHOHUBPROJECT_MOLECULES_HPP


#include "atoms.hpp"
#include "theme.hpp"


namespace Molecules {
    struct User {
        std::string name;
        bool isOnline;
        ImTextureID avatarTexture = 0;
    };

    struct Message {
        std::string author;
        std::string content;
        std::string timestamp;
    };

    inline void UserCard(
        const User& user,
        const Theme& theme,
        const std::function<void()>& onClick = {}
    ) {
        ImGui::BeginGroup();
        if (user.avatarTexture) {
            Atoms::Image(user.avatarTexture, {40, 40}, theme);
        } else {
            Atoms::IconPlaceholder(user.name.empty() ? "?" : user.name.substr(0, 1), {40, 40}, theme, false);
        }
        ImGui::SameLine();

        ImGui::BeginGroup();
        Atoms::Text(user.name, theme, 0);
        if (user.isOnline) {
            ImGui::SameLine();
            Atoms::Text("● Online", theme, 2);
        }
        ImGui::EndGroup();

        ImGui::SameLine();
        Atoms::Button("Czat", theme, {0, 20}, onClick);
        ImGui::EndGroup();
    }

    inline void ChannelListItem(
        const std::string& name,
        const std::string& icon,
        bool isActive,
        const Theme& theme,
        const std::function<void()>& onClick = {}
    ) {
        if (isActive) {
            ImGui::PushStyleColor(ImGuiCol_Text, theme.text);
            ImGui::PushStyleColor(ImGuiCol_Button, theme.surfaceHovered);
        } else {
            ImGui::PushStyleColor(ImGuiCol_Text, theme.textSecondary);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        }

        if (ImGui::Button((icon + " " + name).c_str(), {-1, 0})) {
            if (onClick) onClick();
        }

        ImGui::PopStyleColor(2);
    }

    inline void SearchBar(
        std::string& query,
        const Theme& theme,
        const std::function<void()>& onSearch = {}
    ) {
        Atoms::InputText("", query, theme, {200, 0}, "Szukaj...");
        ImGui::SameLine();
        Atoms::Button("Szukaj", theme, {0, 0}, onSearch);
    }

    inline void Message(const Message& msg, const Theme& theme) {
        Atoms::Text(msg.author + " • " + msg.timestamp, theme, 0);
        Atoms::Text(msg.content, theme, 1);
        ImGui::Spacing();
    }
}


#endif //ECHOHUBPROJECT_MOLECULES_HPP