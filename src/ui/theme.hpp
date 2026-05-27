#ifndef ECHOHUBPROJECT_THEME_HPP
#define ECHOHUBPROJECT_THEME_HPP


// ui/theme/theme.hpp
#pragma once
#include <imgui.h>
#include <string>

struct Theme {
    // --- Kolory ---
    ImVec4 primary = IM_COL32(50, 100, 200, 255);
    ImVec4 primaryHovered = IM_COL32(70, 120, 220, 255);
    ImVec4 background = IM_COL32(30, 30, 30, 255);
    ImVec4 text = IM_COL32(200, 200, 200, 255);
    ImVec4 textDisabled = IM_COL32(100, 100, 100, 255);

    // --- Fonty ---
    ImFont* fontRegular = nullptr;
    ImFont* fontBold = nullptr;
    ImFont* fontIcon = nullptr;  // np. dla ikon FontAwesome

    // --- Spacing ---
    float paddingSmall = 4.0f;
    float paddingMedium = 8.0f;
    float paddingLarge = 16.0f;
    float itemSpacing = 8.0f;

    // --- Metoda do zastosowania theme w ImGui ---
    void apply() const {
        auto& style = ImGui::GetStyle();
        auto& io = ImGui::GetIO();

        // Kolory
        style.Colors[ImGuiCol_Button] = primary;
        style.Colors[ImGuiCol_ButtonHovered] = primaryHovered;
        style.Colors[ImGuiCol_WindowBg] = background;
        style.Colors[ImGuiCol_Text] = text;
        style.Colors[ImGuiCol_TextDisabled] = textDisabled;

        // Fonty (jeśli załadowane)
        if (fontRegular) io.FontDefault = fontRegular;

        // Spacing
        style.WindowPadding = ImVec2(paddingMedium, paddingMedium);
        style.ItemSpacing = ImVec2(itemSpacing, itemSpacing);
    }

    // --- Fabryka motywów ---
    static Theme createDarkTheme() {
        Theme theme;
        theme.primary = IM_COL32(50, 100, 200, 255);
        theme.background = IM_COL32(20, 20, 20, 255);
        theme.text = IM_COL32(220, 220, 220, 255);
        return theme;
    }

    static Theme createLightTheme() {
        Theme theme;
        theme.primary = IM_COL32(0, 100, 200, 255);
        theme.background = IM_COL32(240, 240, 240, 255);
        theme.text = IM_COL32(20, 20, 20, 255);
        return theme;
    }
};

/*
     // 2. Załaduj theme
    ttpk::ui::Theme theme = ttpk::ui::Theme::createDarkTheme();

    // 3. Załaduj fonty (przed ImGui::NewFrame!)
    theme.fontRegular = ImGui::GetIO().Fonts->AddFontFromFileTTF("fonts/roboto.ttf", 16);
    theme.fontBold = ImGui::GetIO().Fonts->AddFontFromFileTTF("fonts/roboto-bold.ttf", 16);

    // 4. W pętli głównej
    while (running) {
        // ...
        ImGui::NewFrame();

        // Zastosuj theme NAJWCZEŚNIEJ (przed rysowaniem UI)
        theme.apply();

        // Rysuj UI (widoki/molekuły/atomy)
        ViewUserList(users);

        // ...
    }

inline bool Button(
        const std::string& label,
        const ttpk::ui::Theme& theme,  // Przekazujemy theme przez const&
        const ImVec2& size = {0, 0},
        const std::function<void()>& onClick = {}
    ) {
        ImGui::PushStyleColor(ImGuiCol_Button, theme.primary);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, theme.primaryHovered);
        bool clicked = ImGui::Button(label.c_str(), size);
        ImGui::PopStyleColor(2);
        if (clicked && onClick) onClick();
        return clicked;
    }
 */

#endif //ECHOHUBPROJECT_THEME_HPP