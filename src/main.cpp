#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include "Account.h"
#include "Billing.h"
#include "Inventory.h"
#include "Requests.h"
#include "Utils.h"

namespace {

constexpr const char* kMedicinesFile = "medicines.txt";
constexpr const char* kBillsFile = "bills.txt";
constexpr const char* kBillCounterFile = "bill_counter.txt";
constexpr const char* kAccountsFile = "accounts.txt";
constexpr const char* kLoginHistoryFile = "login_history.txt";
constexpr const char* kRequestsFile = "requests.txt";
constexpr int kMaxId = 2'000'000'000;
constexpr int kMaxQty = 1'000'000;

void saveOrWarn(const Inventory& inventory) {
    if (!inventory.save())
        std::cout << "Warning: could not save data to " << kMedicinesFile << ".\n";
}

void showOne(const Inventory& inventory, const Medicine& med) {
    inventory.printHeader();
    inventory.printRow(med);
}

void addMedicine(Inventory& inventory) {
    std::cout << "\n--- Add New Medicine ---\n";
    std::string name = util::readLine("Name           : ");

    const std::string lower = util::toLower(name);
    for (const Medicine& m : inventory.all()) {
        if (util::toLower(m.getName()) == lower) {
            std::cout << "A medicine with this name already exists (ID " << m.getId()
                      << "). Use 'Update medicine' to restock it.\n";
            if (!util::readYesNo("Add it as a separate entry anyway?")) return;
            break;
        }
    }

    std::string company = util::readLine("Company        : ");
    std::string category = util::readLine("Category       : ");
    const double price = util::readDouble("Price (Rs.)    : ", 0.0);
    const int qty = util::readInt("Quantity       : ", 0, kMaxQty);
    std::string expiry = util::readDate("Expiry (YYYY-MM-DD): ");

    const Medicine med(inventory.nextId(), std::move(name), std::move(company),
                       std::move(category), price, qty, std::move(expiry));
    inventory.add(med);
    saveOrWarn(inventory);
    std::cout << "Medicine added with ID " << med.getId() << ".\n";
    if (med.isExpired(util::today())) std::cout << "Note: this medicine is already expired.\n";
}

void searchMedicine(Inventory& inventory) {
    std::cout << "\n--- Search Medicine ---\n"
              << "1. By ID\n"
              << "2. By name\n"
              << "3. By category\n"
              << "0. Back\n";
    int found = 0;

    switch (util::readInt("Choice: ", 0, 3)) {
        case 1: {
            const int id = util::readInt("Enter ID: ", 1, kMaxId);
            if (const Medicine* med = inventory.findById(id)) {
                showOne(inventory, *med);
                found = 1;
            }
            break;
        }
        case 2: found = inventory.searchByName(util::readLine("Enter name (or part of it): ")); break;
        case 3: found = inventory.searchByCategory(util::readLine("Enter category: ")); break;
        default: return;
    }

    if (found == 0) std::cout << "No matching medicine found.\n";
    else std::cout << found << " result(s) found.\n";
}

void updateMedicine(Inventory& inventory) {
    std::cout << "\n--- Update Medicine ---\n";
    const int id = util::readInt("Enter medicine ID: ", 1, kMaxId);
    Medicine* med = inventory.findById(id);
    if (med == nullptr) {
        std::cout << "No medicine found with ID " << id << ".\n";
        return;
    }

    bool changed = false;
    while (true) {
        std::cout << '\n';
        showOne(inventory, *med);
        std::cout << "1. Name\n"
                  << "2. Company\n"
                  << "3. Category\n"
                  << "4. Price\n"
                  << "5. Add stock (restock)\n"
                  << "6. Set quantity\n"
                  << "7. Expiry date\n"
                  << "0. Done\n";
        const int choice = util::readInt("What to update: ", 0, 7);
        if (choice == 0) break;

        switch (choice) {
            case 1: med->setName(util::readLine("New name: ")); break;
            case 2: med->setCompany(util::readLine("New company: ")); break;
            case 3: med->setCategory(util::readLine("New category: ")); break;
            case 4: med->setPrice(util::readDouble("New price (Rs.): ", 0.0)); break;
            case 5: med->setQuantity(med->getQuantity() + util::readInt("Units to add: ", 1, kMaxQty)); break;
            case 6: med->setQuantity(util::readInt("New quantity: ", 0, kMaxQty)); break;
            case 7: med->setExpiry(util::readDate("New expiry (YYYY-MM-DD): ")); break;
        }
        changed = true;
        std::cout << "Updated.\n";
    }

    if (changed) {
        saveOrWarn(inventory);
        std::cout << "Changes saved.\n";
    }
}

void deleteMedicine(Inventory& inventory) {
    std::cout << "\n--- Delete Medicine ---\n";
    const int id = util::readInt("Enter medicine ID: ", 1, kMaxId);
    const Medicine* med = inventory.findById(id);
    if (med == nullptr) {
        std::cout << "No medicine found with ID " << id << ".\n";
        return;
    }
    showOne(inventory, *med);
    if (!util::readYesNo("Delete this medicine permanently?")) {
        std::cout << "Delete cancelled.\n";
        return;
    }
    inventory.remove(id);
    saveOrWarn(inventory);
    std::cout << "Medicine deleted.\n";
}

void printTitle(std::string_view subtitle) {
    std::cout << '\n';
    util::printLine('=', 44);
    std::cout << "   PHARMACY / MEDICAL STORE MANAGEMENT\n";
    if (!subtitle.empty()) std::cout << "   " << subtitle << '\n';
    util::printLine('=', 44);
}

void sellerMenu(const Session& session, Inventory& inventory, BillingSystem& billing,
                AccountManager& accounts, const RequestBook& requests) {
    while (true) {
        printTitle("Seller: " + session.username);
        std::cout << " 1. Add medicine\n"
                  << " 2. View all medicines\n"
                  << " 3. Search medicine\n"
                  << " 4. Update medicine / restock\n"
                  << " 5. Delete medicine\n"
                  << " 6. Stock report\n"
                  << " 7. Generate bill\n"
                  << " 8. View bill history\n"
                  << " 9. View medicine requests\n"
                  << "10. View login history\n"
                  << "11. Create account (seller or buyer)\n"
                  << " 0. Logout\n";
        util::printLine('-', 44);

        switch (util::readInt("Enter choice: ", 0, 11)) {
            case 1: addMedicine(inventory); break;
            case 2: inventory.displayAll(); break;
            case 3: searchMedicine(inventory); break;
            case 4: updateMedicine(inventory); break;
            case 5: deleteMedicine(inventory); break;
            case 6: inventory.stockReport(); break;
            case 7: billing.newBill(); break;
            case 8: billing.showHistory(); break;
            case 9:
                requests.showAll();
                if (util::readYesNo("Clear all requests?")) requests.clearAll();
                break;
            case 10: accounts.showLoginHistory(); break;
            case 11: {
                const bool seller = util::readInt("Account type (1 = Seller, 2 = Buyer): ", 1, 2) == 1;
                accounts.signUp(seller ? Role::Seller : Role::Buyer);
                break;
            }
            case 0:
                std::cout << "Logged out.\n";
                return;
        }
        util::pause();
    }
}

void buyerMenu(const Session& session, Inventory& inventory, BillingSystem& billing,
               const RequestBook& requests) {
    while (true) {
        printTitle("Buyer: " + session.username);
        std::cout << " 1. View all medicines\n"
                  << " 2. Search medicine\n"
                  << " 3. Place order\n"
                  << " 4. My orders\n"
                  << " 5. Request a medicine\n"
                  << " 0. Logout\n";
        util::printLine('-', 44);

        switch (util::readInt("Enter choice: ", 0, 5)) {
            case 1: inventory.displayAll(); break;
            case 2: searchMedicine(inventory); break;
            case 3: billing.newBill(session.username); break;
            case 4: billing.showHistory(session.username); break;
            case 5: requests.addRequest(session.username); break;
            case 0:
                std::cout << "Logged out.\n";
                return;
        }
        util::pause();
    }
}

}  // namespace

int main() {
    Inventory inventory(kMedicinesFile);
    inventory.load();
    BillingSystem billing(inventory, kBillsFile, kBillCounterFile);
    AccountManager accounts(kAccountsFile, kLoginHistoryFile);
    const RequestBook requests(kRequestsFile);

    // First run: the store owner sets up the first Seller account.
    if (!accounts.hasAccounts()) {
        printTitle("First-time setup");
        std::cout << "No accounts found. Create the store owner (Seller) account.\n";
        if (!accounts.signUp(Role::Seller)) return 1;
    }

    while (true) {
        printTitle("");
        std::cout << " 1. Login\n"
                  << " 2. Sign up (new buyer)\n"
                  << " 0. Exit\n";
        util::printLine('-', 44);
        const int choice = util::readInt("Enter choice: ", 0, 2);

        if (choice == 0) {
            saveOrWarn(inventory);
            std::cout << "Data saved. Goodbye!\n";
            return 0;
        }

        std::optional<Session> session;
        if (choice == 1) {
            session = accounts.login();
        } else if (accounts.signUp(Role::Buyer)) {
            // Sign-up is buyers only; seller accounts are created by a seller.
            session = accounts.login();
        }

        if (!session) {
            util::pause();
            continue;
        }
        if (session->role == Role::Seller)
            sellerMenu(*session, inventory, billing, accounts, requests);
        else
            buyerMenu(*session, inventory, billing, requests);
    }
}
