#include "users.hpp"
#include <fstream>
#include <filesystem>
#include <algorithm>

const std::string UserManager::USERS_FILE = "data/users.txt";
std::string UserManager::currentUser;

std::vector<std::string> UserManager::loadUsers() {
    std::vector<std::string> users;
    std::ifstream file(USERS_FILE);
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty()) users.push_back(line);
    }
    return users;
}

void UserManager::saveUser(const std::string& username) {
    // Utwórz folder "data/" jeśli nie istnieje
    std::filesystem::create_directory("data");

    std::ofstream file(USERS_FILE, std::ios::app);
    if (file.is_open()) {
        file << username << "\n";
    }
}

void UserManager::setCurrentUser(const std::string& username) {
    currentUser = username;
}

std::string UserManager::getCurrentUser() {
    return currentUser;
}

bool UserManager::userExists(const std::string& username) {
    auto users = loadUsers();
    return std::find(users.begin(), users.end(), username) != users.end();
}