#include "metric_impl/cyclomatic_complexity.hpp"

#include <unistd.h>

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <functional>
#include <iostream>
#include <ranges>
#include <sstream>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace analyser::metric::metric_impl {

namespace rv = std::ranges::views;
using namespace std::string_view_literals;

MetricResult::ValueType CyclomaticComplexityMetric::CalculateImpl(const function::Function &f) const {
    const std::string &ast = f.ast;

    std::string block;
    const std::string block_pattern = "(block";
    size_t block_start = ast.find(block_pattern);
    if (block_start != std::string::npos) {
        size_t open = block_start;
        size_t level = 0;
        size_t i = block_start;
        do {
            if (ast[i] == '(') {
                level++;
            } else if (ast[i] == ')') {
                level--;
            }
            i++;
        } while (i < ast.size() && level > 0);
        block = ast.substr(block_start, i - block_start);
    } else {
        block = ast;
    }

    static const std::array<std::string_view, 12> patterns = {
        "(if_statement"sv,    "(else_clause"sv,   "(elif_clause"sv,      "(while_statement"sv,
        "(for_statement"sv,   "(try_statement"sv, "(except_clause"sv,    "(finally_clause"sv,
        "(match_statement"sv, "(case_clause"sv,   "(assert_statement"sv, "(conditional_expression"sv};

    int complexity = 1;
    for (const auto &pattern : patterns) {
        int count = std::ranges::distance(rv::iota(size_t{0}, block.size()) | rv::filter([&](size_t pos) {
                                              return block.substr(pos, pattern.size()) == pattern;
                                          }));
        complexity += count;
    }
    return complexity;
}

std::string CyclomaticComplexityMetric::Name() const { return "CyclomaticComplexity"; }

}  // namespace analyser::metric::metric_impl
