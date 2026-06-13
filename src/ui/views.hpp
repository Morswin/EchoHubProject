#ifndef ECHOHUBPROJECT_VIEWS_HPP
#define ECHOHUBPROJECT_VIEWS_HPP


#include "molecules.hpp"
#include "atoms.hpp"
#include "theme.hpp"


namespace Views {
    struct Channel {
        std::string name;
        std::string icon;
        bool isTextChannel;
    };

    inline void AuthView(
            std::string& email,
            std::string& password,
            const Theme& theme,
            const std::function<void()>& onLogin = {}
        ) {
        ImGui::SetWindowSize({400, 300});
        ImGui::SetNextWindowPos(
            ImVec2(
                (ImGui::GetIO().DisplaySize.x - 400) * 0.5f,
                (ImGui::GetIO().DisplaySize.y - 300) * 0.5f
            ),
            ImGuiCond_Appearing
        );

        ImGui::Begin("EchoHub - Logowanie", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        Atoms::Text("Witamy ponownie!", theme, 0);
        ImGui::Spacing();

        Atoms::InputText("E-mail", email, theme, {200, 0});
        ImGui::Spacing();
        Atoms::InputText("Hasło", password, theme, {200, 0}, "", true);
        ImGui::Spacing();

        if (Atoms::Button("Zaloguj się", theme, {200, 40}, onLogin)) {}
        ImGui::Spacing();
        Atoms::Button("Zarejestruj konto", theme, {200, 40}, nullptr, true);

        ImGui::End();
    }

    inline void LandingView(
            const Theme& theme,
            const std::function<void()>& onFriendsList = {},
            const std::function<void()>& onConnectToServer = {},
            const std::function<void()>& onCreateServer = {}
        ) {
        ImGui::SetWindowSize({400, 300});
        ImGui::SetNextWindowPos(
            ImVec2(
                (ImGui::GetIO().DisplaySize.x - 400) * 0.5f,
                (ImGui::GetIO().DisplaySize.y - 300) * 0.5f
            ),
            ImGuiCond_Appearing
        );

        ImGui::Begin("EchoHub", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        Atoms::Text("Klon Discord/TeamSpeak", theme, 0);
        ImGui::Spacing();
        Atoms::Text("Nie jesteś połączony z żadnym serwerem. Co chcesz zrobić?", theme, 1);
        ImGui::Spacing();

        if (Atoms::Button("Przejdź do listy znajomych", theme, {200, 40}, onFriendsList)) {}
        ImGui::Spacing();
        if (Atoms::Button("Dołącz do istniejącego serwera", theme, {200, 40}, onConnectToServer)) {}
        ImGui::Spacing();
        if (Atoms::Button("Stwórz nowy serwer", theme, {200, 40}, onCreateServer)) {}

        ImGui::End();
    }

    inline void FriendsListView(
        const std::vector<Molecules::Friend>& friends,
        const Theme& theme
    ) {
        // Grid: [ServerSidebar | SubSidebar | MainContent]
        ImGui::SetWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::Begin("EchoHub - Znajomi", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

        // 1. ServerSidebar (ikony)
        ImGui::BeginChild("ServerSidebar", {72, -1}, true);
        Molecules::ServerIcon("DM", theme, true); // Aktywny
        Molecules::ServerIcon("+", theme, false);
        ImGui::EndChild();
        ImGui::SameLine();

        // 2. SubSidebar (puste, bo to lista znajomych)
        ImGui::BeginChild("SubSidebar", {240, -1}, true);
        Atoms::Text("Prywatne wiadomości", theme, 0);
        ImGui::EndChild();
        ImGui::SameLine();

        // 3. MainContent (lista znajomych)
        ImGui::BeginChild("MainContent");
        Atoms::Text("👥 Znajomi", theme, 0);
        ImGui::Spacing();
        for (const auto& friendData : friends) {
            Molecules::FriendRow(friendData, theme);
            ImGui::Spacing();
        }
        ImGui::EndChild();

        ImGui::End();
    }

    inline void ConnectToNewServerView(
        std::string& serverAddress,
        std::string& nickname,
        const Theme& theme,
        const std::function<void()>& onConnect = {},
        const std::function<void()>& onCancel = {}
    ) {
        ImGui::SetWindowSize({400, 300});
        ImGui::SetWindowPos(
            ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f),
            ImGuiCond_Once
        );
        ImGui::Begin("Dołącz do serwera", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

        Atoms::Text("Dołącz do serwera", theme, 0);
        ImGui::Spacing();

        Atoms::InputText("Adres IP serwera / Kod zaproszenia", serverAddress, theme, {200, 0}, "np. 192.168.1.15:9987");
        ImGui::Spacing();
        Atoms::InputText("Twój Nick", nickname, theme, {200, 0}, "User_C++");
        ImGui::Spacing();

        if (Atoms::Button("Połącz", theme, {200, 40}, onConnect)) {}
        ImGui::Spacing();
        if (Atoms::Button("Anuluj", theme, {200, 40}, onCancel)) {}

        ImGui::End();
    }

    inline void CreateNewServerView(
        std::string& serverName,
        std::string& region,
        const Theme& theme,
        const std::function<void()>& onCreate = {},
        const std::function<void()>& onBack = {}
    ) {
        ImGui::SetWindowSize({400, 300});
        ImGui::SetWindowPos(
            ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f),
            ImGuiCond_Once
        );

        ImGui::Begin("Stwórz serwer", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        Atoms::Text("Stwórz swój serwer", theme, 0);
        ImGui::Spacing();

        Atoms::InputText("Nazwa serwera", serverName, theme, {200, 0}, "Serwer programistyczny");
        ImGui::Spacing();

        // Select dla regionu
        if (ImGui::BeginCombo("Region serwera", region.c_str())) {
            const std::vector<std::string> regions = {"Europa Środkowa (Poland)", "Europa Zachodnia (Frankfurt)"};
            for (const auto& r : regions) {
                bool isSelected = (r == region);
                if (ImGui::Selectable(r.c_str(), isSelected)) {
                    region = r;
                }
                if (isSelected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        ImGui::Spacing();

        if (Atoms::Button("Stwórz", theme, {200, 40}, onCreate)) {}
        ImGui::Spacing();
        if (Atoms::Button("Wróć", theme, {200, 40}, onBack)) {}

        ImGui::End();
    }

    inline void ServerView(
        const std::string& serverName,
        const std::vector<Channel>& channels,
        const std::vector<Molecules::Message>& messages,
        std::string& chatInput,
        const Theme& theme,
        const std::function<void(const std::string&)>& onSendMessage = {}
    ) {
        ImGui::SetWindowSize(ImGui::GetIO().DisplaySize);

        ImGui::Begin("EchoHub - Serwer", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        // 1. ServerSidebar (ikony serwerów)
        ImGui::BeginChild("ServerSidebar", {72, -1}, true);
        Molecules::ServerIcon("DM", theme, false);
        Molecules::ServerIcon(serverName.substr(0, 2), theme, true); // Aktywny
        Molecules::ServerIcon("+", theme, false);
        ImGui::EndChild();
        ImGui::SameLine();

        // 2. SubSidebar (kanały)
        ImGui::BeginChild("SubSidebar", {240, -1}, true);
        Atoms::Text(serverName, theme, 0);
        ImGui::Spacing();

        // Kanały tekstowe
        Atoms::Text("Kanały tekstowe", theme, 1);
        for (const auto& channel : channels) {
            if (channel.isTextChannel) {
                Molecules::ChannelListItem(channel.name, channel.icon, false, theme);
            }
        }
        ImGui::Spacing();

        // Kanały głosowe
        Atoms::Text("Kanały głosowe", theme, 1);
        for (const auto& channel : channels) {
            if (!channel.isTextChannel) {
                Molecules::ChannelListItem(channel.name, channel.icon, false, theme);
            }
        }
        ImGui::EndChild();
        ImGui::SameLine();

        // 3. MainContent (czat)
        ImGui::BeginChild("MainContent");
        // Nagłówek
        Atoms::Text("# ogólny", theme, 0);
        Atoms::Separator(theme);
        ImGui::Spacing();

        // Wiadomości
        for (const auto& msg : messages) {
            Molecules::RenderMessage(msg, theme);
        }

        // Input czatu
        Atoms::InputText("", chatInput, theme, {-1, 0}, "Napisz wiadomość...");
        ImGui::SameLine();
        if (Atoms::Button("Wyślij", theme, {0, 0}, [&]() { if (!chatInput.empty() && onSendMessage) onSendMessage(chatInput); chatInput.clear(); })) {}

        ImGui::EndChild();

        ImGui::End();
    }

    inline void UserSettingsView(
        std::string& username,
        std::string& selectedMic,
        const Theme& theme,
        const std::function<void()>& onSave = {}
    ) {
        ImGui::SetWindowSize({800, 600});
        ImGui::SetWindowPos(
            ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f),
            ImGuiCond_Once
        );

        ImGui::Begin("Ustawienia użytkownika", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        // Sidebar
        ImGui::BeginChild("SettingsSidebar", {240, -1}, true);
        Atoms::Text("Ustawienia użytkownika", theme, 0);
        ImGui::Spacing();
        Molecules::ChannelListItem("Moje konto", "", true, theme);
        Molecules::ChannelListItem("Dźwięk i wideo", "", false, theme);
        Molecules::ChannelListItem("Wygląd", "", false, theme);
        ImGui::Spacing();
        Atoms::Text("Wyloguj się", theme, 3); // 3 = error color
        ImGui::EndChild();
        ImGui::SameLine();

        // Content
        ImGui::BeginChild("SettingsContent");
        Atoms::Text("Moje Konto", theme, 0);
        ImGui::Spacing();

        Atoms::InputText("Nazwa użytkownika", username, theme, {200, 0});
        ImGui::Spacing();

        // Select dla mikrofonu
        if (ImGui::BeginCombo("Urządzenie wejściowe (Mikrofon)", selectedMic.c_str())) {
            const std::vector<std::string> mics = {"Default Microphone Input", "Realtek High Definition Audio"};
            for (const auto& mic : mics) {
                bool isSelected = (mic == selectedMic);
                if (ImGui::Selectable(mic.c_str(), isSelected)) {
                    selectedMic = mic;
                }
                if (isSelected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        ImGui::Spacing();

        if (Atoms::Button("Zapisz zmiany", theme, {200, 40}, onSave)) {}

        ImGui::EndChild();

        ImGui::End();
    }

    inline void ServerSettingsView(
        std::string& serverName,
        std::string& verificationLevel,
        const Theme& theme,
        const std::function<void()>& onSave = {}
    ) {
        ImGui::SetWindowSize({800, 600});
        ImGui::SetWindowPos(
            ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f),
            ImGuiCond_Once
        );

        ImGui::Begin("Ustawienia serwera", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        // Sidebar
        ImGui::BeginChild("SettingsSidebar", {240, -1}, true);
        Atoms::Text("Ustawienia serwera", theme, 0);
        ImGui::Spacing();
        Molecules::ChannelListItem("Przegląd", "", true, theme);
        Molecules::ChannelListItem("Zarządzanie rolami", "", false, theme);
        Molecules::ChannelListItem("Zbanowani użytkownicy", "", false, theme);
        ImGui::Spacing();
        Atoms::Text("Usuń serwer", theme, 3); // 3 = error color
        ImGui::EndChild();
        ImGui::SameLine();

        // Content
        ImGui::BeginChild("SettingsContent");
        Atoms::Text("Przegląd serwera", theme, 0);
        ImGui::Spacing();

        Atoms::InputText("Nazwa serwera", serverName, theme, {200, 0});
        ImGui::Spacing();

        // Select dla weryfikacji
        if (ImGui::BeginCombo("Weryfikacja członków", verificationLevel.c_str())) {
            const std::vector<std::string> levels = {
                "Brak (Dowolny użytkownik może wejść)",
                "Niska (Wymagany zweryfikowany e-mail)"
            };
            for (const auto& level : levels) {
                bool isSelected = (level == verificationLevel);
                if (ImGui::Selectable(level.c_str(), isSelected)) {
                    verificationLevel = level;
                }
                if (isSelected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        ImGui::Spacing();

        if (Atoms::Button("Zapisz zmiany", theme, {200, 40}, onSave)) {}

        ImGui::EndChild();

        ImGui::End();
    }

    inline void ConnectingLoadingView(const Theme& theme) {
        ImGui::SetWindowSize({400, 200});
        ImGui::SetWindowPos(
            ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f),
            ImGuiCond_Once
        );

        ImGui::Begin("Nawiązywanie połączenia", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        // Spinner (złoty)
        ImGui::PushStyleColor(ImGuiCol_Button, theme.primary);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, theme.primaryHovered);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, theme.primaryActive);
        ImGui::Button("##Spinner", {40, 40});
        ImGui::PopStyleColor(3);
        ImGui::SameLine();
        Atoms::Text("Nawiązywanie połączenia...", theme, 0);
        ImGui::Spacing();
        Atoms::Text("Trwa handshake protokołu UDP...", theme, 1);

        ImGui::End();
    }

    inline void ErrorDisconnectedView(
        const Theme& theme,
        const std::function<void()>& onReconnect = {},
        const std::function<void()>& onBackToMain = {}
    ) {
        ImGui::SetWindowSize({400, 200});
        ImGui::SetWindowPos(
            ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f),
            ImGuiCond_Once
        );

        ImGui::Begin("Połączenie przerwane", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        Atoms::Text("Połączenie przerwane", theme, 3); // 3 = error color
        ImGui::Spacing();
        Atoms::Text("Wystąpił błąd sieciowy (WSAECONNRESET). Serwer zdalny zamknął połączenie.", theme, 1);
        ImGui::Spacing();

        if (Atoms::Button("Połącz ponownie", theme, {200, 40}, onReconnect)) {}
        ImGui::Spacing();
        if (Atoms::Button("Wróć do menu głównego", theme, {200, 40}, onBackToMain)) {}

        ImGui::End();
    }
}


#endif //ECHOHUBPROJECT_VIEWS_HPP