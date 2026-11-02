#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

// Console input helpers and small string/date utilities.
namespace util {

// Reads a full line. Re-prompts on empty input unless allowEmpty is true.
// The '|' character is replaced because it is the field separator in data files.
std::string readLine(std::string_view prompt, bool allowEmpty = false);

// Reads an integer in [minVal, maxVal], re-prompting until valid.
int readInt(std::string_view prompt, int minVal, int maxVal);

// Reads a number >= minVal, re-prompting until valid.
double readDouble(std::string_view prompt, double minVal);

// Reads a non-empty password. On Windows the typed characters are shown as '*'.
std::string readPassword(std::string_view prompt);

// Reads y/n and returns true for yes.
bool readYesNo(std::string_view prompt);

// Reads a date in YYYY-MM-DD format, re-prompting until valid.
std::string readDate(std::string_view prompt);

// Parse a whole string as a number; std::nullopt if it is not one.
std::optional<int> parseInt(std::string_view s);
std::optional<double> parseDouble(const std::string& s);

std::string trim(std::string_view s);
std::string toLower(std::string s);

// Splits on delim, keeping empty fields.
std::vector<std::string> split(const std::string& s, char delim);

// Truncates s to fit in a table column of the given width.
std::string fit(const std::string& s, std::size_t width);

bool isValidDate(const std::string& date);
std::string today();         // YYYY-MM-DD
std::string nowTimestamp();  // YYYY-MM-DD HH:MM:SS

void printLine(char ch = '-', int width = 100);
void pause();

}  // namespace util
