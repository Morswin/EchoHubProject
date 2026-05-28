#ifndef ECHOHUBPROJECT_ATOMS_HPP
#define ECHOHUBPROJECT_ATOMS_HPP


#include <imgui.h>
#include <string>
#include <functional>
#include "theme.hpp"


namespace Atoms {
    /// @brief Atom: Przycisk z motywem "Złota Galaktyka".
    /// @param label Tekst na przycisku.
    /// @param theme Motyw (kolory, fonty).
    /// @param size Rozmiar (domyślnie: 0,0 = auto).
    /// @param onClick Callback po kliknięciu.
    /// @param disabled Czy przycisk jest nieaktywny.
    inline bool Button(
        const std::string& label,
        const Theme& theme,
        const ImVec2& size = {0, 0},
        const std::function<void()>& onClick = {},
        bool disabled = false
    ) {
        if (disabled) {
            ImGui::PushStyleColor(ImGuiCol_Button, theme.surface);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, theme.surface);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, theme.surface);
            ImGui::PushStyleColor(ImGuiCol_Text, theme.textDisabled);
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, theme.primary);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, theme.primaryHovered);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, theme.primaryActive);
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 0, 0, 255)); // Czarny tekst na złotym tle
        }

        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, theme.borderRadius);

        bool clicked = ImGui::Button(label.c_str(), size);

        ImGui::PopStyleVar(1);
        ImGui::PopStyleColor(4);

        if (clicked && !disabled && onClick) {
            onClick();
        }
        return clicked;
    }

    /// @brief Atom: Przycisk ikony (np. ✕, ⚙️).
    inline bool IconButton(
        const std::string& icon,
        const Theme& theme,
        const ImVec2& size = {0, 0},
        const std::function<void()>& onClick = {}
    ) {
        ImGui::PushStyleColor(ImGuiCol_Button, theme.surface);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, theme.surfaceHovered);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, theme.primary);
        ImGui::PushStyleColor(ImGuiCol_Text, theme.text);

        bool clicked = ImGui::Button(icon.c_str(), size);

        ImGui::PopStyleColor(4);

        if (clicked && onClick) onClick();
        return clicked;
    }

    inline void InputText(
    const std::string& label,
    std::string& buffer,
    const Theme& theme,
    const ImVec2& size = {0, 0},
    const std::string& hint = "",
    bool password = false
) {
        if (!label.empty()) {
            ImGui::TextColored(theme.text, "%s", label.c_str());
        }

        // --- Style ---
        ImGui::PushStyleColor(ImGuiCol_FrameBg, theme.surface);
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, theme.surfaceHovered);
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, theme.surfaceHovered);
        ImGui::PushStyleColor(ImGuiCol_Border, theme.border);
        ImGui::PushStyleColor(ImGuiCol_Text, theme.text);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, theme.borderRadius);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(theme.paddingSmall, theme.paddingSmall));

        // --- Zarezerwuj miejsce w bufferze (WAŻNE!) ---
        buffer.resize(256); // Zapewnia, że buffer ma co najmniej 256 bajtów
        ImGuiInputTextFlags flags = password ? ImGuiInputTextFlags_Password : 0;

        // --- Wywołanie ImGui::InputText z char* ---
        ImGui::InputText(("##" + label).c_str(), buffer.data(), buffer.capacity(), flags);

        // --- Podpowiedź (placeholder) ---
        if (buffer.empty() && !hint.empty()) {
            ImVec2 cursorPos = ImGui::GetCursorPos();
            ImGui::SetCursorPos(ImVec2(cursorPos.x + theme.paddingSmall, cursorPos.y + theme.paddingSmall));
            ImGui::TextColored(theme.textDisabled, "%s", hint.c_str());
            ImGui::SetCursorPos(cursorPos);
        }

        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(5);
    }

    /// @brief Atom: Tekst z kolorem z motywu.
    /// @param text Tekst do wyświetlenia.
    /// @param theme Motyw.
    /// @param colorType Typ koloru (0 = primary, 1 = secondary, 2 = disabled, 3 = error).
    inline void Text(
        const std::string& text,
        const Theme& theme,
        int colorType = 1  // 0: primary, 1: secondary, 2: disabled, 3: error
    ) {
        ImVec4 color;
        switch (colorType) {
        case 0: color = theme.primary; break;
        case 1: color = theme.text; break;
        case 2: color = theme.textDisabled; break;
        case 3: color = theme.error; break;
        default: color = theme.text;
        }
        ImGui::TextColored(color, "%s", text.c_str());
    }

    /// @brief Atom: Tekst z ikoną (np. "🔊 Poczekalnia").
    inline void IconText(
        const std::string& icon,
        const std::string& text,
        const Theme& theme,
        bool isActive = false
    ) {
        ImVec4 color = isActive ? theme.text : theme.textSecondary;
        ImGui::TextColored(color, "%s %s", icon.c_str(), text.c_str());
    }

    /// @brief Atom: Linia oddzielająca (z motywem).
    /// @param theme Motyw.
    /// @param thickness Grubość linii (domyślnie: 1.0f).
    inline void Separator(const Theme& theme, float thickness = 1.0f) {
        ImGui::PushStyleColor(ImGuiCol_Separator, theme.border);
        // ImGui::PushStyleVar(ImGuiStyleVar_SeparatorThickness, thickness);
        ImGui::PushStyleVar(ImGuiStyleVar_SeparatorSize, thickness);
        ImGui::Separator();
        ImGui::PopStyleVar(1);
        ImGui::PopStyleColor(1);
    }

    /// @brief Atom: Obrazek (ikona serwera, awatar).
    /// @param textureID ID tekstury (z SDL3/ImGui).
    /// @param size Rozmiar obrazka.
    /// @param theme Motyw (nieużywany, ale dla spójności API).
    /// @param isActive Czy ikona jest aktywna (np. wybrany serwer).
    inline void Image(
        ImTextureID textureID,
        const ImVec2& size,
        const Theme& theme,
        bool isActive = false
    ) {
        if (isActive) {
            ImGui::PushStyleColor(ImGuiCol_Border, theme.primary);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, theme.borderRadius * 2);
        }

        ImGui::Image(textureID, size);

        if (isActive) {
            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor(1);
        }
    }

    /// @brief Atom: Placeholder dla ikony (kolorowe koło).
    /// @param label Tekst w środku (np. "DM", "+").
    /// @param size Rozmiar.
    /// @param theme Motyw.
    /// @param isActive Czy ikona jest aktywna.
    inline void IconPlaceholder(
        const std::string& label,
        const ImVec2& size,
        const Theme& theme,
        bool isActive = false
    ) {
        if (isActive) {
            ImGui::PushStyleColor(ImGuiCol_Button, theme.primary);
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 0, 0, 255));
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, theme.surface);
            ImGui::PushStyleColor(ImGuiCol_Text, theme.textSecondary);
        }

        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, size.x / 2);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0);

        ImGui::Button(label.c_str(), size);

        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(2);
    }
}


#endif //ECHOHUBPROJECT_ATOMS_HPP