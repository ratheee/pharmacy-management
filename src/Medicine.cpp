#include "Medicine.h"

#include <iomanip>
#include <sstream>
#include <utility>

#include "Utils.h"

Medicine::Medicine(int medId, std::string medName, std::string medCompany,
                   std::string medCategory, double medPrice, int medQuantity,
                   std::string medExpiry)
    : id(medId), name(std::move(medName)), company(std::move(medCompany)),
      category(std::move(medCategory)), price(medPrice), quantity(medQuantity),
      expiry(std::move(medExpiry)) {}

std::string Medicine::serialize() const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    oss << id << '|' << name << '|' << company << '|' << category << '|'
        << price << '|' << quantity << '|' << expiry;
    return oss.str();
}

std::optional<Medicine> Medicine::parse(const std::string& line) {
    // trim() also drops a trailing '\r' left by files edited on Windows
    const std::vector<std::string> f = util::split(util::trim(line), '|');
    if (f.size() != 7) return std::nullopt;

    const auto id = util::parseInt(f[0]);
    const auto price = util::parseDouble(f[4]);
    const auto qty = util::parseInt(f[5]);
    if (!id || *id <= 0 || !price || *price < 0 || !qty || *qty < 0 || !util::isValidDate(f[6]))
        return std::nullopt;

    return Medicine(*id, f[1], f[2], f[3], *price, *qty, f[6]);
}
