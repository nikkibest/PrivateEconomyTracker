#ifndef NUMBERS_H
#define NUMBERS_H

// Add this function to your gui.cpp or a utility header
#include <sstream>
#include <iomanip>
#include <locale>

// Custom numpunct for European formatting
struct euro_num_punct : std::numpunct<char> {
    char do_decimal_point() const override { return ','; }
    char do_thousands_sep() const override { return '.'; }
    std::string do_grouping() const override { return "\3"; }
};

inline std::string format_euro(double value) {
    std::stringstream ss;
    ss.imbue(std::locale(std::locale::classic(), new euro_num_punct));
    ss << std::fixed << std::setprecision(2) << value;
    return ss.str();
}

#endif // NUMBERS_H