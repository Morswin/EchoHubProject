#ifndef ECHOHUBPROJECT_APP_STATE_HPP
#define ECHOHUBPROJECT_APP_STATE_HPP

#include <SDL3/SDL.h>
#include <vector>
#include <string>
#include "ui/theme.hpp"
#include "ui/molecules.hpp"
#include "ui/views.hpp"

class AppState {
public:
    // --- SDL ---
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    // --- View State ---
    EViewState currentView = EViewState::AUTH_VIEW;

    // --- User Data ---
    std::string username;
    std::string email;
    std::string password;
    std::string confirmPassword;

    // --- Server Data ---
    std::string serverAddress;
    std::string nickname;
    std::string serverName;
    std::string region;
    std::string selectedMic = "Default Microphone Input";
    std::string verificationLevel = "Brak (Dowolny użytkownik może wejść)";

    // --- Chat Data ---
    std::string chatInput;
    std::vector<Molecules::Friend> friends = {
        {"JanKowalski", true},
        {"Programista99", false}
    };
    std::vector<Views::Channel> channels = {
        {"ogólny", "#", true},
        {"pomoc-kod", "#", true},
        {"Poczekalnia", "🔊", false},
        {"Pokój gier", "🔊", false}
    };
    std::vector<Molecules::Message> messages = {
        {"Admin", "Cześć! Witamy na klonie Discorda napisanym w C++.", "Dzisiaj o 12:00"},
        {"User_C++", "Wygląda super, czat tekstowy i enumy działają stabilnie!", "Dzisiaj o 12:05"}
    };

    // --- Theme ---
    Theme theme = Theme::createGoldGalaxyTheme();

    // --- Gettery/Settery ---
    [[nodiscard]] EViewState getCurrentView() const { return currentView; }
    void setCurrentView(EViewState view) { currentView = view; }

    [[nodiscard]] const std::string& getUsername() const { return username; }
    void setUsername(const std::string& newUsername) { username = newUsername; }
};

#endif //ECHOHUBPROJECT_APP_STATE_HPP