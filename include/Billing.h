#pragma once

#include <string>

#include "DynamicArray.h"
#include "Inventory.h"

struct BillItem {
    int medicineId = 0;
    std::string name;
    double unitPrice = 0.0;
    int quantity = 0;

    [[nodiscard]] double amount() const { return unitPrice * quantity; }
};

// A customer's cart, stored in a DynamicArray.
class Bill {
public:
    // Adds qty of a medicine; merges with an existing line for the same medicine.
    void addItem(const Medicine& med, int qty);
    bool removeItem(int medicineId);
    [[nodiscard]] int quantityOf(int medicineId) const;

    [[nodiscard]] const DynamicArray<BillItem>& items() const { return lines; }
    [[nodiscard]] std::size_t size() const { return lines.size(); }
    [[nodiscard]] bool isEmpty() const { return lines.empty(); }

    [[nodiscard]] double subtotal() const;
    [[nodiscard]] double tax() const;
    [[nodiscard]] double total() const { return subtotal() + tax(); }

    void printCart() const;
    [[nodiscard]] std::string format(int billNo, const std::string& customer,
                                     const std::string& timestamp) const;

private:
    DynamicArray<BillItem> lines;
};

// Runs the billing workflow: builds a cart, updates stock and saves bills.
class BillingSystem {
public:
    BillingSystem(Inventory& inventory, std::string billsFile, std::string counterFile);

    // Asks for the customer's name when customerName is empty.
    void newBill(const std::string& customerName = "");
    // Shows all bills, or only those for one customer when a filter is given.
    void showHistory(const std::string& customerFilter = "") const;

private:
    void addToCart(Bill& bill);
    void removeFromCart(Bill& bill);
    bool checkout(Bill& bill, const std::string& customer);

    [[nodiscard]] int lastBillNo() const;
    void saveBillNo(int billNo) const;

    Inventory& inventory;
    std::string billsFile;
    std::string counterFile;
};
