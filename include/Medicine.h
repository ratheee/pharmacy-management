#pragma once

#include <optional>
#include <string>
#include <utility>

class Medicine {
public:
    Medicine() = default;
    Medicine(int medId, std::string medName, std::string medCompany, std::string medCategory,
             double medPrice, int medQuantity, std::string medExpiry);

    [[nodiscard]] int getId() const { return id; }
    [[nodiscard]] const std::string& getName() const { return name; }
    [[nodiscard]] const std::string& getCompany() const { return company; }
    [[nodiscard]] const std::string& getCategory() const { return category; }
    [[nodiscard]] double getPrice() const { return price; }
    [[nodiscard]] int getQuantity() const { return quantity; }
    [[nodiscard]] const std::string& getExpiry() const { return expiry; }

    void setName(std::string v) { name = std::move(v); }
    void setCompany(std::string v) { company = std::move(v); }
    void setCategory(std::string v) { category = std::move(v); }
    void setPrice(double v) { price = v; }
    void setQuantity(int v) { quantity = v; }
    void setExpiry(std::string v) { expiry = std::move(v); }

    // Dates are YYYY-MM-DD, so string comparison gives chronological order.
    [[nodiscard]] bool isExpired(const std::string& today) const { return expiry < today; }
    [[nodiscard]] bool isLowStock(int threshold) const { return quantity <= threshold; }

    // One record per line: id|name|company|category|price|quantity|expiry
    [[nodiscard]] std::string serialize() const;
    // Returns std::nullopt for a malformed line.
    static std::optional<Medicine> parse(const std::string& line);

private:
    int id = 0;
    std::string name;
    std::string company;
    std::string category;
    double price = 0.0;
    int quantity = 0;
    std::string expiry;
};
