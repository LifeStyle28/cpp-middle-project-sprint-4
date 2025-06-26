#include "metric_impl/parameters_count.hpp"

#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <ranges>
#include <sstream>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "utils.hpp"

namespace analyser::metric::metric_impl {

namespace rv = std::ranges::views;
namespace rs = std::ranges;
using namespace std::string_view_literals;

constexpr size_t IDENTIFIER_LEN = 11;

MetricResult::ValueType CountParametersMetric::CalculateImpl(const function::Function &f) const {
    const std::string &ast = f.ast;
    const std::string parameters_marker = "parameters: (parameters";

    size_t params_start = ast.find(parameters_marker);
    if (params_start == std::string::npos) {
        return 0;
    }

    size_t open_brace = ast.find('(', params_start);
    if (open_brace == std::string::npos) {
        return 0;
    }

    size_t brace_count = 1;
    size_t pos = open_brace + 1;

    while (pos < ast.size() && brace_count > 0) {
        if (ast[pos] == '(') {
            brace_count++;
        } else if (ast[pos] == ')') {
            brace_count--;
        }
        pos++;
    }

    if (brace_count > 0) {
        return 0;
    }

    auto params_block = ast.substr(open_brace, pos - open_brace);
    auto identifier_positions = rv::iota(size_t{0}, params_block.size()) | rv::filter([&](size_t pos) {
                                    if (pos + IDENTIFIER_LEN > params_block.size()) {
                                        return false;
                                    }
                                    if (params_block.substr(pos, IDENTIFIER_LEN) != "(identifier") {
                                        return false;
                                    }

                                    size_t prev_open = params_block.rfind('(', pos);
                                    return prev_open == std::string::npos || prev_open == pos;
                                });
    int count = 0;

    static const std::vector<std::string_view> patterns = {"(identifier"sv,         "(default_parameter"sv,
                                                           "(typed_parameter"sv,    "(typed_default_parameter"sv,
                                                           "(list_splat_pattern"sv, "(dictionary_splat_pattern"sv};

    // Создаем view для итерации по символам с отслеживанием уровня вложенности
    auto char_with_level = rv::iota(size_t{0}, params_block.size()) |
                           rv::transform([&](size_t i) { return std::make_pair(params_block[i], i); });

    int level = 0;

    for (auto [ch, pos] : char_with_level) {
        if (ch == '(') {
            // Проверяем, начинается ли здесь один из паттернов на первом уровне
            for (const auto &pattern : patterns) {
                if (params_block.substr(pos, pattern.size()) == pattern && level == 1) {
                    count++;
                    break;
                }
            }
            level++;
        } else if (ch == ')') {
            level--;
        }
    }

    return count;
}

std::string CountParametersMetric::Name() const { return "ParametersCount"; }

}  // namespace analyser::metric::metric_impl
