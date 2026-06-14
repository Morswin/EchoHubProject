#ifndef ECHOHUBPROJECT_VIEW_STATES_HPP
#define ECHOHUBPROJECT_VIEW_STATES_HPP

/**
 * @brief A way to describe a view state in 1 enum.
 *
 * This doesn't include the data, and that's why ViewState class exists.
 * I'll use this docstring for planning out the initial set of views.
 * TODO: Remove this part of docstring once the initial set of views is implemented. This part of docstring may not be needed by then.
 *
 * LANDING_VIEW:
 *  Whenever you open the app, this is the entry view for the program.
 * CONNECT_TO_NEW_SERVER_VIEW:
 *  Here user may provide data required for a connecting to an already existing server.
 * CREATE_NEW_SERVER_VIEW:
 *  Here user may set up a new server.
 * SERVER_VIEW:
 *  Displays relevant info about server that the user is currently connected to.
 *  Contains channel selection section and chat section (if a text channel has been selected)
 * USER_SETTINGS_VIEW:
 *  Here user should be able to define or configure their profile.
 * SERVER_SETTINGS_VIEW:
 *  Here a server owner will be able to configure server settings.
 */
enum class EViewState {
    AUTH_VIEW,                  // Logowanie i rejestracja
    LANDING_VIEW,               // Ekran startowy / Dashboard
    FRIENDS_LIST_VIEW,          // Lista znajomych i czaty DM (Direct Messages)
    CONNECT_TO_NEW_SERVER_VIEW, // Dołączanie do serwera przez kod/IP
    CREATE_NEW_SERVER_VIEW,     // Tworzenie nowego serwera
    CONNECTING_LOADING_VIEW,    // Ekran ładowania podczas połączenia
    SERVER_VIEW,                // Główny widok serwera (kanały + czat)
    USER_SETTINGS_VIEW,         // Ustawienia profilu / audio
    SERVER_SETTINGS_VIEW,       // Zarządzanie serwerem (role, nazwa)
    ERROR_DISCONNECTED_VIEW,    // Błędy sieciowe i utrata połączenia
    REGISTER_VIEW,
    CREATE_CHANNEL_VIEW,        // Tworzenie nowego kanału
    DIRECT_MESSAGE_VIEW,        // Czat prywatny (DM) z znajomym
};

#endif //ECHOHUBPROJECT_VIEW_STATES_HPP