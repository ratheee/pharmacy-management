#include "Inventory.h"

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <system_error>
#include <utility>

namespace fs = std::filesystem;

Inventory::Inventory(std::string dataFile) : fileName(std::move(dataFile)) {}

void Inventory::load() {
    std::ifstream in(fileName);
    if (!in) return;  // no data file yet: start with an empty inventory

    items.clear();
    std::string line;
    int skipped = 0;
    while (std::getline(in, line)) {
        if (util::trim(line).empty()) continue;
        auto med = Medicine::parse(line);
        if (!med || findById(med->getId()) != nullptr) {
            ++skipped;
            continue;
        }
        items.push_back(std::move(*med));
    }
    if (skipped > 0)
        std::cout << "Warning: skipped " << skipped << " invalid or duplicate record(s) in "
                  << fileName << ".\n";
}

bool Inventory::save() const {
    // Write to a temp file first so a failed write cannot wipe the existing data.
    const std::string tmpName = fileName + ".tmp";
    {
        std::ofstream out(tmpName, std::ios::trunc);
        if (!out) return false;
        for (const Medicine& med : items) out << med.serialize() << '\n';
        if (!out) return false;
    }
    std::error_code ec;
    fs::rename(tmpName, fileName, ec);
    if (ec) {
        // Some Windows standard libraries won't rename over an existing file.
        fs::remove(fileName, ec);
        fs::rename(tmpName, fileName, ec);
    }
    return !ec;
}

int Inventory::nextId() const {
    int maxId = 100;  // IDs start at 101
    for (const Medicine& med : items)
        if (med.getId() > maxId) maxId = med.getId();
    return maxId + 1;
}

void Inventory::add(const Medicine& med) { items.push_back(med); }

bool Inventory::remove(int id) {
    for (std::size_t i = 0; i < items.size(); ++i) {
        if (items[i].getId() == id) {
            items.removeAt(i);
            return true;
        }
    }
    return false;
}

Medicine* Inventory::findById(int id) {
    for (Medicine& med : items)
        if (med.getId() == id) return &med;
    return nullptr;
}

const Medicine* Inventory::findById(int id) const {
    for (const Medicine& med : items)
        if (med.getId() == id) return &med;
    return nullptr;
}

void Inventory::printHeader() const {
    util::printLine('-', kTableWidth);
    std::cout << std::left << std::setw(6) << "ID" << std::setw(22) << "Name"
              << std::setw(16) << "Company" << std::setw(14) << "Category"
              << std::right << std::setw(10) << "Price" << std::setw(7) << "Qty"
              << "  " << std::left << std::setw(12) << "Expiry" << "Status\n";
    util::printLine('-', kTableWidth);
}

void Inventory::printRow(const Medicine& med) const {
    std::string status = "OK";
    if (med.isExpired(util::today())) status = "EXPIRED";
    else if (med.getQuantity() == 0) status = "OUT";
    else if (med.isLowStock(kLowStockThreshold)) status = "LOW";

    std::cout << std::left << std::setw(6) << med.getId()
              << std::setw(22) << util::fit(med.getName(), 21)
              << std::setw(16) << util::fit(med.getCompany(), 15)
              << std::setw(14) << util::fit(med.getCategory(), 13)
              << std::right << std::fixed << std::setprecision(2)
              << std::setw(10) << med.getPrice() << std::setw(7) << med.getQuantity()
              << "  " << std::left << std::setw(12) << med.getExpiry() << status << '\n';
}

void Inventory::displayAll() const {
    if (items.empty()) {
        std::cout << "No medicines in stock.\n";
        return;
    }
    printWhere([](const Medicine&) { return true; });
    std::cout << "Total medicines: " << items.size() << '\n';
}

int Inventory::searchByName(const std::string& text) const {
    const std::string needle = util::toLower(text);
    return printWhere([&needle](const Medicine& med) {
        return util::toLower(med.getName()).find(needle) != std::string::npos;
    });
}

int Inventory::searchByCategory(const std::string& text) const {
    const std::string needle = util::toLower(text);
    return printWhere([&needle](const Medicine& med) {
        return util::toLower(med.getCategory()).find(needle) != std::string::npos;
    });
}

void Inventory::stockReport() const {
    const std::string todayStr = util::today();
    int totalUnits = 0, lowCount = 0, outCount = 0, expiredCount = 0;
    double totalValue = 0.0;

    for (const Medicine& m : items) {
        totalUnits += m.getQuantity();
        totalValue += m.getPrice() * m.getQuantity();
        if (m.isExpired(todayStr)) ++expiredCount;
        if (m.getQuantity() == 0) ++outCount;
        else if (m.isLowStock(kLowStockThreshold)) ++lowCount;
    }

    std::cout << "\n===== STOCK REPORT (" << todayStr << ") =====\n"
              << "Medicines listed   : " << items.size() << '\n'
              << "Total units        : " << totalUnits << '\n'
              << "Total stock value  : Rs. " << std::fixed << std::setprecision(2)
              << totalValue << '\n'
              << "Low stock (<= " << kLowStockThreshold << ")   : " << lowCount << '\n'
              << "Out of stock       : " << outCount << '\n'
              << "Expired            : " << expiredCount << '\n';

    if (lowCount + outCount + expiredCount == 0) {
        std::cout << "\nAll medicines are in stock and within expiry.\n";
        return;
    }

    std::cout << "\nItems needing attention:\n";
    printWhere([&todayStr](const Medicine& m) {
        return m.isExpired(todayStr) || m.isLowStock(kLowStockThreshold);
    });
}
