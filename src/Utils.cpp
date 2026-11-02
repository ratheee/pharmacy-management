#include "Utils.h"

#include <array>
#include <cctype>
#include <charconv>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

#ifdef _WIN32
#include <conio.h>
#include <io.h>
#endif

namespace util {

namespace {

[[noreturn]] void exitOnClosedInput() {
    std::cout << "\nInput closed. Exiting.\n";
    std::exit(0);
}

// Removes terminal escape sequences and control characters. Terminals such as
// Git Bash wrap pasted text in ESC[200~ ... ESC[201~, and arrow keys arrive as
// ESC[A etc. when the program is not attached to a real console.
std::string stripControlChars(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (std::size_t i = 0; i < s.size(); ++i) {
        const unsigned char c = static_cast<unsigned char>(s[i]);
        if (c == 0x1B) {
            // CSI sequence: ESC [ parameters... final byte in '@'..'~'
            if (i + 1 < s.size() && s[i + 1] == '[') {
                i += 2;
                while (i < s.size() && !(s[i] >= '@' && s[i] <= '~')) ++i;
            } else {
                ++i;  // two-character escape such as ESC O
            }
            continue;
        }
        if (c < 0x20 || c == 0x7F) continue;
        out += static_cast<char>(c);
    }
    return out;
}

// Reads one line from stdin with escape sequences removed and spaces trimmed.
std::string readCleanLine() {
    std::string line;
    if (!std::getline(std::cin, line)) exitOnClosedInput();
    return trim(stripControlChars(line));
}

std::string formatNow(const char* fmt) {
    std::time_t t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm local{};
#if defined(_MSC_VER)
    localtime_s(&local, &t);
#elif defined(_WIN32)
    local = *std::localtime(&t);  // MinGW may lack localtime_s/_r; fine in a single-threaded app
#else
    localtime_r(&t, &local);
#endif
    std::ostringstream oss;
    oss << std::put_time(&local, fmt);
    return oss.str();
}

}  // namespace

std::string trim(std::string_view s) {
    const auto start = s.find_first_not_of(" \t\r\n");
    if (start == std::string_view::npos) return "";
    const auto end = s.find_last_not_of(" \t\r\n");
    return std::string(s.substr(start, end - start + 1));
}

std::string toLower(std::string s) {
    for (char& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}

std::vector<std::string> split(const std::string& s, char delim) {
    std::vector<std::string> parts;
    std::size_t start = 0;
    while (true) {
        const auto pos = s.find(delim, start);
        if (pos == std::string::npos) {
            parts.push_back(s.substr(start));
            return parts;
        }
        parts.push_back(s.substr(start, pos - start));
        start = pos + 1;
    }
}

std::string fit(const std::string& s, std::size_t width) {
    if (s.size() <= width) return s;
    if (width <= 3) return s.substr(0, width);
    return s.substr(0, width - 3) + "...";
}

std::optional<int> parseInt(std::string_view s) {
    int value = 0;
    const char* first = s.data();
    const char* last = s.data() + s.size();
    auto [ptr, ec] = std::from_chars(first, last, value);
    if (s.empty() || ec != std::errc() || ptr != last) return std::nullopt;
    return value;
}

std::optional<double> parseDouble(const std::string& s) {
    if (s.empty()) return std::nullopt;
    char* end = nullptr;
    double value = std::strtod(s.c_str(), &end);
    if (*end != '\0') return std::nullopt;
    return value;
}

std::string readLine(std::string_view prompt, bool allowEmpty) {
    while (true) {
        std::cout << prompt;
        std::string line = readCleanLine();
        for (char& c : line)
            if (c == '|') c = '/';
        if (!line.empty() || allowEmpty) return line;
        std::cout << "  Input cannot be empty.\n";
    }
}

int readInt(std::string_view prompt, int minVal, int maxVal) {
    while (true) {
        auto value = parseInt(readLine(prompt));
        if (!value) {
            std::cout << "  Invalid number, try again.\n";
        } else if (*value < minVal || *value > maxVal) {
            std::cout << "  Enter a number between " << minVal << " and " << maxVal << ".\n";
        } else {
            return *value;
        }
    }
}

double readDouble(std::string_view prompt, double minVal) {
    while (true) {
        auto value = parseDouble(readLine(prompt));
        if (!value) {
            std::cout << "  Invalid number, try again.\n";
        } else if (*value < minVal) {
            std::cout << "  Value must be at least " << minVal << ".\n";
        } else {
            return *value;
        }
    }
}

std::string readPassword(std::string_view prompt) {
    while (true) {
        std::cout << prompt << std::flush;
        std::string pass;
#ifdef _WIN32
        // _getch reads the keyboard directly, so only use it for a real console;
        // redirected input (a file or pipe) is read as a normal line.
        const bool console = _isatty(_fileno(stdin)) != 0;
        while (console) {
            int ch = _getch();
            if (ch == '\r' || ch == '\n') break;
            if (ch == 3) std::exit(0);  // Ctrl+C
            if (ch == 0 || ch == 0xE0) {
                _getch();  // skip the second byte of arrow / function keys
                continue;
            }
            if (ch == '\b') {
                if (!pass.empty()) {
                    pass.pop_back();
                    std::cout << "\b \b" << std::flush;
                }
                continue;
            }
            if (std::isprint(ch)) {
                pass += static_cast<char>(ch);
                std::cout << '*' << std::flush;
            }
        }
        if (console) {
            std::cout << '\n';
        } else {
            pass = readCleanLine();
        }
#else
        pass = readCleanLine();
#endif
        if (!pass.empty()) return pass;
        std::cout << "  Password cannot be empty.\n";
    }
}

bool readYesNo(std::string_view prompt) {
    const std::string fullPrompt = std::string(prompt) + " (y/n): ";
    while (true) {
        const std::string ans = toLower(readLine(fullPrompt));
        if (ans == "y" || ans == "yes") return true;
        if (ans == "n" || ans == "no") return false;
        std::cout << "  Please enter y or n.\n";
    }
}

bool isValidDate(const std::string& date) {
    if (date.size() != 10 || date[4] != '-' || date[7] != '-') return false;
    auto year = parseInt(std::string_view(date).substr(0, 4));
    auto month = parseInt(std::string_view(date).substr(5, 2));
    auto day = parseInt(std::string_view(date).substr(8, 2));
    if (!year || !month || !day) return false;
    if (*year < 1900 || *month < 1 || *month > 12 || *day < 1) return false;

    constexpr std::array<int, 12> daysInMonth{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int maxDay = daysInMonth[static_cast<std::size_t>(*month - 1)];
    const bool leap = (*year % 4 == 0 && *year % 100 != 0) || *year % 400 == 0;
    if (*month == 2 && leap) maxDay = 29;
    return *day <= maxDay;
}

std::string readDate(std::string_view prompt) {
    while (true) {
        std::string date = readLine(prompt);
        if (isValidDate(date)) return date;
        std::cout << "  Invalid date. Use YYYY-MM-DD (e.g. 2027-03-31).\n";
    }
}

std::string today() { return formatNow("%Y-%m-%d"); }

std::string nowTimestamp() { return formatNow("%Y-%m-%d %H:%M:%S"); }

void printLine(char ch, int width) {
    std::cout << std::string(static_cast<std::size_t>(width > 0 ? width : 0), ch) << '\n';
}

void pause() {
    std::cout << "\nPress Enter to continue...";
    std::string dummy;
    std::getline(std::cin, dummy);
}

}  // namespace util
