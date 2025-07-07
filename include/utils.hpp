#pragma once

#include <cctype>
#include <charconv>
#include <stdexcept>
#include <string>
#include <string_view>

inline int ToInt(std::string_view value) {
    value.remove_prefix(std::min(value.find_first_not_of(" "), value.size()));
    value.remove_suffix(std::min(value.find_first_not_of(" "), value.size()));

    int result{};
    auto [parse_end_ptr, error_code] = std::from_chars(value.begin(), value.end(), result);
    if (error_code != std::errc{} || parse_end_ptr != value.data() + value.size()) {
        throw std::invalid_argument("Cannot convert '" + std::string(value) + "' to integral");
    }
    return result;
}
