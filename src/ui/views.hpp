#ifndef ECHOHUBPROJECT_VIEWS_HPP
#define ECHOHUBPROJECT_VIEWS_HPP


#include "molecules.hpp"
#include "atoms.hpp"
#include "theme.hpp"


namespace Views {
    inline void AuthView(
            std::string& email,
            std::string& password,
            const Theme& theme
        ) {
        ImGui::Begin("EchoHub - Logowanie", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
        ImGui::SetWindowSize({400, 300});
        ImGui::SetWindowPos(
            ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f),
            ImGuiCond_Once
        );

        Atoms::Text("Witamy ponownie!", theme, 0);
        ImGui::Spacing();

        Atoms::InputText("E-mail", email, theme, {200, 0});
        ImGui::Spacing();
        Atoms::InputText("Hasło", password, theme, {200, 0}, "", true);
        ImGui::Spacing();

        if (Atoms::Button("Zaloguj się", theme, {200, 40})) {
            // Obsługa logowania
        }
        ImGui::Spacing();
        Atoms::Button("Zarejestruj konto", theme, {200, 40}, nullptr, true);

        ImGui::End();
    }
}


#endif //ECHOHUBPROJECT_VIEWS_HPP