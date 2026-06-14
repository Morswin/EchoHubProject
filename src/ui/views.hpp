#ifndef ECHOHUBPROJECT_VIEWS_HPP
#define ECHOHUBPROJECT_VIEWS_HPP


#include "molecules.hpp"
#include "atoms.hpp"
#include "theme.hpp"
#include "view_states.hpp"
#include "users.hpp"


namespace Views {
    struct Channel {
        std::string name;
        std::string icon;
        bool isTextChannel;
    };

    inline void AuthView(
            std::string& username,
            const Theme& theme,
            const std::function<void()>& onLogin = {},
            const std::function<void()>& onRegister = {}
        ) {
        ImGui::SetNextWindowSize({400, 400});
        ImGui::SetNextWindowPos(
            ImVec2(
                (ImGui::GetIO().DisplaySize.x - 400) * 0.5f,
                (ImGui::GetIO().DisplaySize.y - 400) * 0.5f
            ),
            ImGuiCond_Appearing
        );
        ImGui::Begin("EchoHub - Wybierz użytkownika", nullptr,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove
        );

        Atoms::Text("Witamy w EchoHub!", theme, 0);
        ImGui::Spacing();

        // Lista istniejących użytkowników
        static std::vector<std::string> users = UserManager::loadUsers();
        static int selectedUserIndex = -1;

        if (ImGui::BeginCombo("Wybierz użytkownika", selectedUserIndex >= 0 ? users[selectedUserIndex].c_str() : "Nowy użytkownik")) {
            for (int i = 0; i < users.size(); i++) {
                bool isSelected = (selectedUserIndex == i);
                if (ImGui::Selectable(users[i].c_str(), isSelected)) {
                    selectedUserIndex = i;
                    username = users[i];
                }
                if (isSelected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        ImGui::Spacing();
        Atoms::InputText("Nazwa użytkownika", username, theme, {200, 0}, "Wprowadź swoją nazwę");
        ImGui::Spacing();

        if (Atoms::Button("Zaloguj się", theme, {200, 40}, [&]() {
            if (!username.empty()) {
                if (selectedUserIndex >= 0) {
                    username = users[selectedUserIndex];
                }
                UserManager::setCurrentUser(username);
                if (onLogin) onLogin();
            }
        })) {}

        ImGui::Spacing();
        if (Atoms::Button("Zarejestruj nowego użytkownika", theme, {200, 40}, onRegister)) {}

        ImGui::End();
    }

    inline void LandingView(
            const Theme& theme,
            const std::function<void()>& onFriendsList = {},
            const std::function<void()>& onConnectToServer = {},
            const std::function<void()>& onCreateServer = {}
        ) {
        ImGui::SetNextWindowSize({400, 300});
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
        const Theme& theme,
        const std::function<void()>& onCreateServer = {},
        const std::function<void(const std::string&)>& onConnectToServer = {},
        const std::function<void()>& onDirectMessage = {}
    ) {
        // Grid: [ServerSidebar | SubSidebar | MainContent]
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::SetNextWindowPos({0, 0});
        ImGui::Begin("EchoHub - Znajomi", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

        // 1. ServerSidebar (ikony)
        ImGui::BeginChild("ServerSidebar", {72, -1}, true);
        Molecules::ServerIcon("DM", theme, true, onDirectMessage);
        Molecules::ServerIcon("+", theme, false, onCreateServer);
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
        ImGui::SetNextWindowSize({400, 300});
        ImGui::SetNextWindowPos(
            ImVec2(
                (ImGui::GetIO().DisplaySize.x - 400) * 0.5f,
                (ImGui::GetIO().DisplaySize.y - 300) * 0.5f
            ),
            ImGuiCond_Appearing
        );
        // ImGui::SetWindowPos(
        //     ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f),
        //     ImGuiCond_Once
        // );
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
        ImGui::SetNextWindowSize({400, 300});
        // ImGui::SetNextWindowPos(
        //     ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f),
        //     ImGuiCond_Once
        // );
        ImGui::SetNextWindowPos(
            ImVec2(
                (ImGui::GetIO().DisplaySize.x - 400) * 0.5f,
                (ImGui::GetIO().DisplaySize.y - 300) * 0.5f
            ),
            ImGuiCond_Appearing
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
        const std::string& currentChannel,
        const std::string& currentVoiceChannel,
        bool isVoiceActive,
        const std::vector<std::string>& usersInChannel,
        const std::function<void(const std::string&)>& onSendMessage = {},
        const std::function<void()>& onCreateServer = {},
        const std::function<void(const std::string&)>& onConnectToServer = {},
        const std::function<void()>& onDirectMessage = {},
        const std::function<void(const std::string&, bool)>& onSwitchChannel = {},
        const std::function<void()>& onCreateChannel = {}
    ) {
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::SetNextWindowPos({0, 0});

        ImGui::Begin("EchoHub - Serwer", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        // 1. ServerSidebar (ikony serwerów)
        ImGui::BeginChild("ServerSidebar", {72, -1}, true);

        Molecules::ServerIcon("DM", theme, false, [&]() {
            if (onDirectMessage) onDirectMessage();
        });
        Molecules::ServerIcon(serverName.substr(0, 2), theme, true);
        Molecules::ServerIcon("+", theme, false, onCreateServer);
        ImGui::EndChild();
        ImGui::SameLine();

        // 2. SubSidebar (kanały)
        ImGui::BeginChild("SubSidebar", {240, -1}, true);
        ImGui::BeginGroup();
        Atoms::Text(serverName, theme, 0);
        ImGui::SameLine();
        if (Atoms::Button("+", theme, {20, 20}, onCreateChannel)) {}
        ImGui::EndGroup();
        ImGui::Spacing();

        // Kanały tekstowe
        Atoms::Text("Kanały tekstowe", theme, 1);
        for (const auto& channel : channels) {
            if (channel.isTextChannel) {
                bool isActive = (currentChannel == channel.name);
                Molecules::ChannelListItem(channel.name, channel.icon, isActive, theme,
                    [&, channelName = channel.name]() {
                        if (onSwitchChannel) onSwitchChannel(channelName, true);
                    }
                );
            }
        }
        ImGui::Spacing();

        // Kanały głosowe
        Atoms::Text("Kanały głosowe", theme, 1);
        for (const auto& channel : channels) {
            if (!channel.isTextChannel) {
                // Check if this is the current voice channel
                bool isActiveVoice = (currentVoiceChannel == channel.name);
                Molecules::ChannelListItem(channel.name, channel.icon, isActiveVoice, theme, 
                    [&, channelName = channel.name]() {
                        if (onSwitchChannel) onSwitchChannel(channelName, false);
                    }
                );
            }
        }
        ImGui::EndChild();
        ImGui::SameLine();

        // 3. MainContent (czat)
        ImGui::BeginChild("MainContent");
        // Nagłówek z indykatorem voice i nazwą kanału
        ImGui::BeginGroup();
        Atoms::Text("# " + currentChannel, theme, 0);
        ImGui::SameLine();
        if (isVoiceActive) {
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
            Atoms::Text("🎤", theme, 0);
            ImGui::PopStyleColor();
            ImGui::SameLine();
            Atoms::Text("Aktywny", theme, 2);
        }
        ImGui::EndGroup();
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
        ImGui::SameLine();

        // 4. UserPanel (użytkownicy w kanale)
        ImGui::BeginChild("UserPanel", {200, -1}, true);
        Atoms::Text("Użytkownicy", theme, 0);
        ImGui::Spacing();
        
        if (usersInChannel.empty()) {
            Atoms::Text("Brak użytkowników", theme, 1);
        } else {
            for (const auto& user : usersInChannel) {
                Molecules::FriendRow({user, true}, theme);
                ImGui::Spacing();
            }
        }
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

    inline void ConnectingLoadingView(const Theme& theme, const std::string& status = "") {
        ImGui::SetNextWindowSize({400, 200});
        ImGui::SetNextWindowPos(
            ImVec2(
                (ImGui::GetIO().DisplaySize.x - 400) * 0.5f,  // (1920-400)/2 = 760
                (ImGui::GetIO().DisplaySize.y - 200) * 0.5f   // (1080-200)/2 = 440
            ),
            ImGuiCond_Appearing
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
        
        if (!status.empty()) {
            Atoms::Text(status.c_str(), theme, 1);
        } else {
            Atoms::Text("Trwa handshake protokołu UDP...", theme, 1);
        }

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

    inline void CreateChannelView(
        std::string& channelName,
        bool& isTextChannel,
        const Theme& theme,
        const std::function<void(const std::string&, bool)>& onCreate = {},
        const std::function<void()>& onCancel = {}
    ) {
        ImGui::SetNextWindowSize({400, 250});
        ImGui::SetNextWindowPos(
            ImVec2(
                (ImGui::GetIO().DisplaySize.x - 400) * 0.5f,
                (ImGui::GetIO().DisplaySize.y - 250) * 0.5f
            ),
            ImGuiCond_Appearing
        );
        ImGui::Begin("Utwórz nowy kanał", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        
        Atoms::Text("Utwórz nowy kanał", theme, 0);
        ImGui::Spacing();
        
        Atoms::InputText("Nazwa kanału", channelName, theme, {200, 0}, "np. pomoc-techniczna");
        ImGui::Spacing();
        
        // Radio buttons for channel type
        if (ImGui::RadioButton("Kanał tekstowy", isTextChannel)) {
            isTextChannel = true;
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Kanał głosowy", !isTextChannel)) {
            isTextChannel = false;
        }
        ImGui::Spacing();
        
        ImGui::BeginGroup();
        if (Atoms::Button("Utwórz", theme, {150, 40}, [&]() {
            if (!channelName.empty() && onCreate) {
                onCreate(channelName, isTextChannel);
            }
        })) {}
        ImGui::SameLine();
        if (Atoms::Button("Anuluj", theme, {150, 40}, onCancel)) {}
        ImGui::EndGroup();
        
        ImGui::End();
    }

    inline void RegisterView(
    std::string& username,
    const Theme& theme,
    const std::function<void()>& onRegister = {},
    const std::function<void()>& onBackToLogin = {}
) {
        ImGui::SetNextWindowSize({400, 300});
        ImGui::SetNextWindowPos(
            ImVec2(
                (ImGui::GetIO().DisplaySize.x - 400) * 0.5f,
                (ImGui::GetIO().DisplaySize.y - 300) * 0.5f
            ),
            ImGuiCond_Appearing
        );
        ImGui::Begin("EchoHub - Nowy użytkownik", nullptr,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove
        );

        Atoms::Text("Stwórz nowe konto", theme, 0);
        ImGui::Spacing();
        Atoms::InputText("Nazwa użytkownika", username, theme, {200, 0}, "Twoja nazwa wyświetlana");
        ImGui::Spacing();

        if (Atoms::Button("Zarejestruj się", theme, {200, 40}, [&]() {
            if (!username.empty() && !UserManager::userExists(username)) {
                UserManager::saveUser(username);
                UserManager::setCurrentUser(username);
                if (onRegister) onRegister();
            }
        })) {}

        ImGui::Spacing();
        if (Atoms::Button("Mam już konto", theme, {200, 40}, onBackToLogin)) {}

        ImGui::End();
    }
}


#endif //ECHOHUBPROJECT_VIEWS_HPP