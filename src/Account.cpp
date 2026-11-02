#include "Account.h"

#include <cctype>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <utility>
#include <vector>

#include "Utils.h"

namespace {

constexpr int kMaxLoginAttempts = 3;
constexpr int kMaxSignupAttempts = 3;
constexpr std::size_t kMinPasswordLength = 4;

// 64-bit FNV-1a. Deterministic across compilers, so the accounts file stays
// valid whichever compiler built the program. It keeps plain passwords out of
// the file but is NOT a cryptographic password hash.
std::string hashPassword(const std::string& username, const std::string& password) {
    const std::string input = util::toLower(username) + ":pharmacy:" + password;
    std::uint64_t hash = 14695981039346656037ULL;
    for (char ch : input) {
        hash ^= static_cast<unsigned char>(ch);
        hash *= 1099511628211ULL;
    }
    std::ostringstream oss;
    oss << std::hex << std::setw(16) << std::setfill('0') << hash;
    return oss.str();
}

bool isValidUsername(const std::string& name) {
    if (name.size() < 3 || name.size() > 20) return false;
    for (char ch : name)
        if (!std::isalnum(static_cast<unsigned char>(ch)) && ch != '_') return false;
    return true;
}

}  // namespace

AccountManager::AccountManager(std::string accounts, std::string history)
    : accountsFile(std::move(accounts)), historyFile(std::move(history)) {}

bool AccountManager::hasAccounts() const {
    std::ifstream in(accountsFile);
    std::string line;
    while (std::getline(in, line))
        if (!util::trim(line).empty()) return true;
    return false;
}

std::optional<AccountManager::Record> AccountManager::findAccount(const std::string& username) const {
    std::ifstream in(accountsFile);
    const std::string wanted = util::toLower(username);
    std::string line;
    while (std::getline(in, line)) {
        const std::vector<std::string> f = util::split(util::trim(line), '|');
        if (f.size() != 3) continue;
        if (util::toLower(f[0]) == wanted) {
            const Role role = (f[1] == roleName(Role::Seller)) ? Role::Seller : Role::Buyer;
            return Record{f[0], role, f[2]};
        }
    }
    return std::nullopt;
}

std::optional<Session> AccountManager::signUp(Role role) {
    std::cout << "\n--- Create " << roleName(role) << " Account ---\n"
              << "Username: 3-20 characters, letters, digits or _\n";

    std::string username;
    while (true) {
        username = util::readLine("Username: ");
        if (!isValidUsername(username))
            std::cout << "  Invalid username.\n";
        else if (findAccount(username))
            std::cout << "  That username is taken. Try another.\n";
        else
            break;
    }

    for (int attempt = 0; attempt < kMaxSignupAttempts; ++attempt) {
        const std::string password = util::readPassword("Password (min 4 chars): ");
        if (password.size() < kMinPasswordLength) {
            std::cout << "  Password is too short.\n";
            continue;
        }
        if (util::readPassword("Confirm password: ") != password) {
            std::cout << "  Passwords do not match. Try again.\n";
            continue;
        }

        std::ofstream out(accountsFile, std::ios::app);
        if (!out) {
            std::cout << "Error: could not write to " << accountsFile << ".\n";
            return std::nullopt;
        }
        out << username << '|' << roleName(role) << '|' << hashPassword(username, password) << '\n';
        std::cout << roleName(role) << " account '" << username << "' created.\n";
        return Session{username, role};
    }
    std::cout << "Too many failed attempts. Account not created.\n";
    return std::nullopt;
}

std::optional<Session> AccountManager::login() {
    std::cout << "\n--- Login ---\n";
    for (int attempt = 1; attempt <= kMaxLoginAttempts; ++attempt) {
        const std::string username = util::readLine("Username: ");
        const std::string password = util::readPassword("Password: ");

        if (auto account = findAccount(username);
            account && account->hash == hashPassword(username, password)) {
            Session session{account->username, account->role};
            recordLogin(session);
            std::cout << "Welcome, " << session.username << "! Logged in as "
                      << roleName(session.role) << ".\n";
            return session;
        }
        std::cout << "Wrong username or password. Attempts left: "
                  << (kMaxLoginAttempts - attempt) << '\n';
    }
    std::cout << "Too many failed attempts. Try again later.\n";
    return std::nullopt;
}

void AccountManager::recordLogin(const Session& session) const {
    if (std::ofstream out(historyFile, std::ios::app); out)
        out << session.username << '|' << roleName(session.role) << '|'
            << util::nowTimestamp() << '\n';
}

void AccountManager::showLoginHistory() const {
    std::ifstream in(historyFile);
    std::string line;
    int shown = 0;
    while (std::getline(in, line)) {
        const std::vector<std::string> f = util::split(util::trim(line), '|');
        if (f.size() != 3) continue;
        if (shown++ == 0) {
            util::printLine('-', 50);
            std::cout << std::left << std::setw(22) << "Username" << std::setw(9) << "Role"
                      << "Login time\n";
            util::printLine('-', 50);
        }
        std::cout << std::left << std::setw(22) << f[0] << std::setw(9) << f[1] << f[2] << '\n';
    }
    if (shown == 0) std::cout << "No logins recorded yet.\n";
    else util::printLine('-', 50);
}
