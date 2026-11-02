#pragma once

#include <string>

// Medicine requests from buyers for items the store does not stock.
// Stored one per line: timestamp|username|medicine|quantity
class RequestBook {
public:
    explicit RequestBook(std::string file);

    void addRequest(const std::string& username) const;
    void showAll() const;
    void clearAll() const;

private:
    std::string fileName;
};
