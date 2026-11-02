#pragma once

#include <optional>
#include <string>
#include <string_view>

enum class Role { Buyer, Seller };

constexpr std::string_view roleName(Role role) {
    return role == Role::Seller ? "Seller" : "Buyer";
}

struct Session {
    std::string username;
    Role role = Role::Buyer;
};

// Stores user accounts (username|role|passwordHash) and the login history.
// Only a password hash is stored, never the plain password.
class AccountManager {
public:
    AccountManager(std::string accountsFile, std::string historyFile);

    [[nodiscard]] bool hasAccounts() const;

    // Interactive sign-up. Returns the new account's session, or nullopt on failure.
    std::optional<Session> signUp(Role role);
    // Interactive login with up to 3 attempts. Returns the session, or nullopt on failure.
    std::optional<Session> login();

    void showLoginHistory() const;

private:
    struct Record {
        std::string username;
        Role role;
        std::string hash;
    };

    [[nodiscard]] std::optional<Record> findAccount(const std::string& username) const;
    void recordLogin(const Session& session) const;

    std::string accountsFile;
    std::string historyFile;
};
