#ifndef ECHOHUBPROJECT_THEME_HPP
#define ECHOHUBPROJECT_THEME_HPP


#include <imgui.h>
#include <string>
#include <functional>

struct Theme {
    // ===== KOLORY =====
    ImVec4 primary = ImVec4(1.0f, 215.0f/255.0f, 0.0f, 1.0f);       // #FFD700
    ImVec4 primaryHovered = ImVec4(218.0f/255.0f, 165.0f/255.0f, 32.0f/255.0f, 1.0f); // #DAA520
    ImVec4 primaryActive = ImVec4(184.0f/255.0f, 134.0f/255.0f, 11.0f/255.0f, 1.0f); // #B8860B
    ImVec4 background = ImVec4(10.0f/255.0f, 10.0f/255.0f, 15.0f/255.0f, 1.0f);    // #0A0A0F
    ImVec4 surface = ImVec4(20.0f/255.0f, 20.0f/255.0f, 30.0f/255.0f, 0.9f);       // #14141E
    ImVec4 surfaceHovered = ImVec4(40.0f/255.0f, 40.0f/255.0f, 60.0f/255.0f, 0.9f); // #28283C
    ImVec4 text = ImVec4(1.0f, 215.0f/255.0f, 0.0f, 1.0f);             // #FFD700
    ImVec4 textSecondary = ImVec4(192.0f/255.0f, 192.0f/255.0f, 192.0f/255.0f, 1.0f); // #C0C0C0
    ImVec4 textDisabled = ImVec4(128.0f/255.0f, 128.0f/255.0f, 142.0f/255.0f, 1.0f); // #80848E
    ImVec4 error = ImVec4(242.0f/255.0f, 63.0f/255.0f, 67.0f/255.0f, 1.0f);      // #F23F43
    ImVec4 border = ImVec4(255.0f/255.0f, 215.0f/255.0f, 0.0f, 100.0f/255.0f);   // Przezroczysty złoty
    ImVec4 online = ImVec4(35.0f/255.0f, 165.0f/255.0f, 90.0f/255.0f, 1.0f);     // #23A55A
    ImVec4 offline = ImVec4(128.0f/255.0f, 132.0f/255.0f, 142.0f/255.0f, 1.0f); // #80848E

    // ===== FONTY =====
    ImFont* fontRegular = nullptr;
    ImFont* fontBold = nullptr;
    ImFont* fontIcon = nullptr;   // Dla ikon (np. FontAwesome)

    // ===== SPACING =====
    float paddingSmall = 4.0f;
    float paddingMedium = 8.0f;
    float paddingLarge = 16.0f;
    float itemSpacing = 8.0f;
    float borderRadius = 4.0f;
    float windowBorderSize = 1.0f;

    // ===== METODY =====
    /// @brief Zastosuj motyw do ImGui (wywoływać na początku każdej klatki, przed ImGui::NewFrame).
    void apply() const {
        auto& style = ImGui::GetStyle();
        auto& io = ImGui::GetIO();

        // --- Kolory ImGui ---
        style.Colors[ImGuiCol_Text] = text;
        style.Colors[ImGuiCol_TextDisabled] = textDisabled;
        style.Colors[ImGuiCol_WindowBg] = background;
        style.Colors[ImGuiCol_ChildBg] = surface;
        style.Colors[ImGuiCol_PopupBg] = surface;
        style.Colors[ImGuiCol_Border] = border;
        style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

        // --- Frame (InputText, Select, itp.) ---
        style.Colors[ImGuiCol_FrameBg] = surface;
        style.Colors[ImGuiCol_FrameBgHovered] = surfaceHovered;
        style.Colors[ImGuiCol_FrameBgActive] = surfaceHovered;

        // --- Button ---
        style.Colors[ImGuiCol_Button] = primary;
        style.Colors[ImGuiCol_ButtonHovered] = primaryHovered;
        style.Colors[ImGuiCol_ButtonActive] = primaryActive;
        // style.Colors[ImGuiCol_ButtonText] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f); // Czarny tekst na złotym buttonie

        // --- TitleBg (Nagłówki okien) ---
        style.Colors[ImGuiCol_TitleBg] = surface;
        style.Colors[ImGuiCol_TitleBgActive] = surface;
        style.Colors[ImGuiCol_TitleBgCollapsed] = surface;
        // style.Colors[ImGuiCol_TitleText] = text;

        // --- Scrollbar ---
        style.Colors[ImGuiCol_ScrollbarBg] = surface;
        style.Colors[ImGuiCol_ScrollbarGrab] = primary;
        style.Colors[ImGuiCol_ScrollbarGrabHovered] = primaryHovered;
        style.Colors[ImGuiCol_ScrollbarGrabActive] = primaryActive;

        // --- Checkbox, Radio, Slider ---
        style.Colors[ImGuiCol_CheckMark] = primary;
        style.Colors[ImGuiCol_SliderGrab] = primary;
        style.Colors[ImGuiCol_SliderGrabActive] = primaryActive;

        // --- Tabs ---
        style.Colors[ImGuiCol_Tab] = surface;
        style.Colors[ImGuiCol_TabHovered] = surfaceHovered;
        style.Colors[ImGuiCol_TabActive] = primary;
        // style.Colors[ImGuiCol_TabText] = textSecondary;
        // style.Colors[ImGuiCol_TabTextActive] = text;

        // --- Fonty ---
        if (fontRegular) io.FontDefault = fontRegular;
        // if (fontBold) io.Fonts->AddFontFromMemoryTTF(/* ... */); // Opcjonalnie

        // --- Spacing & Rounding ---
        style.WindowPadding = ImVec2(paddingMedium, paddingMedium);
        style.FramePadding = ImVec2(paddingSmall, paddingSmall);
        style.ItemSpacing = ImVec2(itemSpacing, itemSpacing);
        style.ItemInnerSpacing = ImVec2(paddingSmall, paddingSmall);
        style.TouchExtraPadding = ImVec2(0, 0);
        style.IndentSpacing = paddingMedium;
        style.ScrollbarSize = 12.0f;
        style.GrabMinSize = 10.0f;

        // --- Border & Rounding ---
        style.WindowBorderSize = windowBorderSize;
        style.ChildBorderSize = windowBorderSize;
        style.PopupBorderSize = windowBorderSize;
        style.FrameBorderSize = windowBorderSize;
        style.TabBorderSize = windowBorderSize;
        style.WindowRounding = borderRadius;
        style.ChildRounding = borderRadius;
        style.FrameRounding = borderRadius;
        style.PopupRounding = borderRadius;
        style.ScrollbarRounding = borderRadius;
        style.GrabRounding = borderRadius;
        style.TabRounding = borderRadius;
    }

    /// @brief Fabryka: Stwórz motyw "Złota Galaktyka".
    static Theme createGoldGalaxyTheme() {
        Theme theme;
        // Kolory są już zdefiniowane w konstruktorze, ale możesz tutaj dostosować.
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