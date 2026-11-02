#pragma once

#include <cstddef>
#include <string>

#include "DynamicArray.h"
#include "Medicine.h"
#include "Utils.h"

// Holds all medicines in a DynamicArray and persists them to a text file.
class Inventory {
public:
    static constexpr int kLowStockThreshold = 10;
    static constexpr int kTableWidth = 100;

    explicit Inventory(std::string dataFile);

    Inventory(const Inventory&) = delete;
    Inventory& operator=(const Inventory&) = delete;

    void load();
    [[nodiscard]] bool save() const;

    [[nodiscard]] int nextId() const;
    void add(const Medicine& med);
    bool remove(int id);

    Medicine* findById(int id);
    const Medicine* findById(int id) const;

    [[nodiscard]] std::size_t size() const { return items.size(); }
    [[nodiscard]] const DynamicArray<Medicine>& all() const { return items; }

    void printHeader() const;
    void printRow(const Medicine& med) const;
    void displayAll() const;

    // Prints every medicine for which matches(med) is true; returns how many.
    template <typename Predicate>
    int printWhere(Predicate matches) const {
        int found = 0;
        for (const Medicine& med : items) {
            if (!matches(med)) continue;
            if (found++ == 0) printHeader();
            printRow(med);
        }
        if (found > 0) util::printLine('-', kTableWidth);
        return found;
    }

    // Case-insensitive partial match on name / category. Return the number of matches.
    int searchByName(const std::string& text) const;
    int searchByCategory(const std::string& text) const;
    void stockReport() const;

private:
    DynamicArray<Medicine> items;
    std::string fileName;
};
