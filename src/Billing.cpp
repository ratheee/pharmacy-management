#include "Billing.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string_view>
#include <utility>

#include "Utils.h"

namespace {

constexpr double kGstRate = 0.05;
constexpr int kFirstBillNo = 1001;
constexpr int kBillWidth = 52;
constexpr int kMaxId = 2'000'000'000;
constexpr std::string_view kStoreName = "CITY MEDICAL STORE";

std::string centered(std::string_view text, int width) {
    const int pad = (width - static_cast<int>(text.size())) / 2;
    return std::string(static_cast<std::size_t>(pad > 0 ? pad : 0), ' ') + std::string(text);
}

}  // namespace

// ---------------------------------------------------------------- Bill

void Bill::addItem(const Medicine& med, int qty) {
    for (BillItem& line : lines) {
        if (line.medicineId == med.getId()) {
            line.quantity += qty;
            return;
        }
    }
    lines.push_back(BillItem{med.getId(), med.getName(), med.getPrice(), qty});
}

bool Bill::removeItem(int medicineId) {
    for (std::size_t i = 0; i < lines.size(); ++i) {
        if (lines[i].medicineId == medicineId) {
            lines.removeAt(i);
            return true;
        }
    }
    return false;
}

int Bill::quantityOf(int medicineId) const {
    for (const BillItem& line : lines)
        if (line.medicineId == medicineId) return line.quantity;
    return 0;
}

double Bill::subtotal() const {
    double sum = 0.0;
    for (const BillItem& line : lines) sum += line.amount();
    return sum;
}

double Bill::tax() const { return subtotal() * kGstRate; }

void Bill::printCart() const {
    if (isEmpty()) {
        std::cout << "Cart is empty.\n";
        return;
    }
    util::printLine('-', kBillWidth);
    std::cout << std::left << std::setw(8) << "ID" << std::setw(20) << "Medicine"
              << std::right << std::setw(5) << "Qty" << std::setw(9) << "Rate"
              << std::setw(10) << "Amount" << '\n';
    util::printLine('-', kBillWidth);
    std::cout << std::fixed << std::setprecision(2);
    for (const BillItem& it : lines) {
        std::cout << std::left << std::setw(8) << it.medicineId
                  << std::setw(20) << util::fit(it.name, 19)
                  << std::right << std::setw(5) << it.quantity
                  << std::setw(9) << it.unitPrice << std::setw(10) << it.amount() << '\n';
    }
    util::printLine('-', kBillWidth);
    std::cout << std::right << std::setw(42) << "Subtotal: " << std::setw(10) << subtotal() << '\n';
}

std::string Bill::format(int billNo, const std::string& customer,
                         const std::string& timestamp) const {
    std::ostringstream out;
    const std::string heavy(kBillWidth, '='), light(kBillWidth, '-');

    out << heavy << '\n' << centered(kStoreName, kBillWidth) << '\n' << heavy << '\n'
        << "Bill No : " << billNo << '\n'
        << "Date    : " << timestamp << '\n'
        << "Customer: " << customer << '\n'
        << light << '\n'
        << std::left << std::setw(4) << "No" << std::setw(24) << "Medicine"
        << std::right << std::setw(5) << "Qty" << std::setw(9) << "Rate"
        << std::setw(10) << "Amount" << '\n'
        << light << '\n'
        << std::fixed << std::setprecision(2);

    int lineNo = 1;
    for (const BillItem& it : lines) {
        out << std::left << std::setw(4) << lineNo++ << std::setw(24) << util::fit(it.name, 23)
            << std::right << std::setw(5) << it.quantity << std::setw(9) << it.unitPrice
            << std::setw(10) << it.amount() << '\n';
    }

    const std::string gstLabel =
        "GST (" + std::to_string(static_cast<int>(kGstRate * 100 + 0.5)) + "%) : ";

    out << light << '\n'
        << std::setw(42) << "Subtotal : " << std::setw(10) << subtotal() << '\n'
        << std::setw(42) << gstLabel << std::setw(10) << tax() << '\n'
        << std::setw(42) << "Total (Rs.) : " << std::setw(10) << total() << '\n'
        << heavy << '\n'
        << centered("Thank you! Get well soon.", kBillWidth) << '\n'
        << heavy << '\n';
    return out.str();
}

// ---------------------------------------------------------------- BillingSystem

BillingSystem::BillingSystem(Inventory& inv, std::string bills, std::string counter)
    : inventory(inv), billsFile(std::move(bills)), counterFile(std::move(counter)) {}

int BillingSystem::lastBillNo() const {
    std::ifstream in(counterFile);
    int last = 0;
    if (in >> last && last >= kFirstBillNo) return last;
    return kFirstBillNo - 1;
}

void BillingSystem::saveBillNo(int billNo) const {
    std::ofstream out(counterFile, std::ios::trunc);
    out << billNo << '\n';
}

void BillingSystem::addToCart(Bill& bill) {
    const int id = util::readInt("Enter medicine ID (0 to go back): ", 0, kMaxId);
    if (id == 0) return;

    const Medicine* med = inventory.findById(id);
    if (med == nullptr) {
        std::cout << "No medicine found with ID " << id << ".\n";
        return;
    }
    if (med->isExpired(util::today())) {
        std::cout << med->getName() << " expired on " << med->getExpiry()
                  << " and cannot be sold.\n";
        return;
    }
    const int inCart = bill.quantityOf(id);
    const int available = med->getQuantity() - inCart;
    if (available <= 0) {
        std::cout << med->getName() << " is out of stock"
                  << (inCart > 0 ? " (all units already in cart).\n" : ".\n");
        return;
    }

    std::cout << med->getName() << " - Rs. " << std::fixed << std::setprecision(2)
              << med->getPrice() << " each, " << available << " available.\n";
    const int qty = util::readInt("Quantity: ", 1, available);
    bill.addItem(*med, qty);
    std::cout << "Added " << qty << " x " << med->getName() << " to cart.\n";
}

void BillingSystem::removeFromCart(Bill& bill) {
    if (bill.isEmpty()) {
        std::cout << "Cart is empty.\n";
        return;
    }
    bill.printCart();
    const int id = util::readInt("Enter medicine ID to remove (0 to go back): ", 0, kMaxId);
    if (id == 0) return;
    if (bill.removeItem(id)) std::cout << "Item removed from cart.\n";
    else std::cout << "That medicine is not in the cart.\n";
}

bool BillingSystem::checkout(Bill& bill, const std::string& customer) {
    if (bill.isEmpty()) {
        std::cout << "Cart is empty. Add items before checkout.\n";
        return false;
    }

    const int billNo = lastBillNo() + 1;
    const std::string billText = bill.format(billNo, customer, util::nowTimestamp());
    std::cout << '\n' << billText;
    if (!util::readYesNo("Confirm and generate this bill?")) return false;

    for (const BillItem& it : bill.items()) {
        if (Medicine* med = inventory.findById(it.medicineId))
            med->setQuantity(med->getQuantity() - it.quantity);
    }
    if (!inventory.save()) std::cout << "Warning: could not save updated stock to file.\n";

    if (std::ofstream out(billsFile, std::ios::app); out) {
        out << billText << '\n';
        saveBillNo(billNo);
        std::cout << "Bill #" << billNo << " generated and saved to " << billsFile << ".\n";
    } else {
        std::cout << "Warning: could not write bill to " << billsFile << ".\n";
    }
    return true;
}

void BillingSystem::newBill(const std::string& customerName) {
    if (inventory.size() == 0) {
        std::cout << "Inventory is empty. Add medicines before billing.\n";
        return;
    }

    const std::string customer =
        customerName.empty() ? util::readLine("Customer name: ") : customerName;
    Bill bill;

    while (true) {
        std::cout << "\n--- Billing: " << customer << " | items: " << bill.size()
                  << " | subtotal: Rs. " << std::fixed << std::setprecision(2)
                  << bill.subtotal() << " ---\n"
                  << "1. Add medicine\n"
                  << "2. Remove medicine\n"
                  << "3. View cart\n"
                  << "4. Checkout\n"
                  << "0. Cancel bill\n";

        switch (util::readInt("Choice: ", 0, 4)) {
            case 1: addToCart(bill); break;
            case 2: removeFromCart(bill); break;
            case 3: bill.printCart(); break;
            case 4:
                if (checkout(bill, customer)) return;
                break;
            case 0:
                if (bill.isEmpty() || util::readYesNo("Discard this bill?")) {
                    std::cout << "Bill cancelled. Stock unchanged.\n";
                    return;
                }
                break;
        }
    }
}

void BillingSystem::showHistory(const std::string& customerFilter) const {
    // Bills are stored as blocks of lines separated by a blank line.
    std::ifstream in(billsFile);
    const std::string wanted = "Customer: " + customerFilter;
    std::string line, block;
    bool blockMatches = false;
    int shown = 0;

    while (true) {
        const bool more = static_cast<bool>(std::getline(in, line));
        if (more && !util::trim(line).empty()) {
            block += line + '\n';
            if (util::trim(line) == wanted) blockMatches = true;
            continue;
        }
        if (!block.empty() && (customerFilter.empty() || blockMatches)) {
            std::cout << block << '\n';
            ++shown;
        }
        block.clear();
        blockMatches = false;
        if (!more) break;
    }
    if (shown == 0) std::cout << "No bills found.\n";
}
