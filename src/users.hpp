#ifndef ECHOHUBPROJECT_USERS_HPP
#define ECHOHUBPROJECT_USERS_HPP


#include <vector>
#include <string>


class UserManager {
public:
    // Wczytuje listę użytkowników z pliku
    static std::vector<std::string> loadUsers();

    // Zapisuje nowego użytkownika do pliku
    static void saveUser(const std::string& username);

    // Ustawia aktywnego użytkownika
    static void setCurrentUser(const std::string& username);

    // Zwraca aktywnego użytkownika
    static std::string getCurrentUser();

    // Sprawdza, czy użytkownik istnieje
    static bool userExists(const std::string& username);

private:
    static const std::string USERS_FILE;
    static std::string currentUser;
};


#endif //ECHOHUBPROJECT_USERS_HPP