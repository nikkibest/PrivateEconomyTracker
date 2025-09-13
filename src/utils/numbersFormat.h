#ifndef NUMBERSFORMAT_H
#define NUMBERSFORMAT_H

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

inline std::string format_euro(double value, int precision = 2) {
    std::stringstream ss;
    ss.imbue(std::locale(std::locale::classic(), new euro_num_punct));
    ss << std::fixed << std::setprecision(precision) << value;
    return ss.str();
}

inline int format_euro_implot(double value, char* buff, int size, void*) {
    std::string euro = format_euro(value, 0); // No decimals for axis labels
    strncpy(buff, euro.c_str(), size);
    buff[size - 1] = '\0';
    return (int)euro.size();
}

#endif // NUMBERSFORMAT_H