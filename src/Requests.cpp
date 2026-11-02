#include "Requests.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <utility>
#include <vector>

#include "Utils.h"

RequestBook::RequestBook(std::string file) : fileName(std::move(file)) {}

void RequestBook::addRequest(const std::string& username) const {
    std::cout << "\n--- Request a Medicine ---\n";
    const std::string medicine = util::readLine("Medicine name: ");
    const int qty = util::readInt("Quantity needed: ", 1, 100'000);

    std::ofstream out(fileName, std::ios::app);
    if (!out) {
        std::cout << "Error: could not save the request.\n";
        return;
    }
    out << util::nowTimestamp() << '|' << username << '|' << medicine << '|' << qty << '\n';
    std::cout << "Request submitted. The store will be notified.\n";
}

void RequestBook::showAll() const {
    std::ifstream in(fileName);
    std::string line;
    int shown = 0;
    while (std::getline(in, line)) {
        const std::vector<std::string> f = util::split(util::trim(line), '|');
        if (f.size() != 4) continue;
        if (shown++ == 0) {
            util::printLine('-', 80);
            std::cout << std::left << std::setw(21) << "Requested at" << std::setw(18) << "Buyer"
                      << std::setw(30) << "Medicine" << "Qty\n";
            util::printLine('-', 80);
        }
        std::cout << std::left << std::setw(21) << f[0] << std::setw(18) << util::fit(f[1], 17)
                  << std::setw(30) << util::fit(f[2], 29) << f[3] << '\n';
    }
    if (shown == 0) std::cout << "No pending medicine requests.\n";
    else util::printLine('-', 80);
}

void RequestBook::clearAll() const {
    std::ofstream out(fileName, std::ios::trunc);
    std::cout << "All requests cleared.\n";
}
