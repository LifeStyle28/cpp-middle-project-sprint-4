#include "metric_impl/code_lines_count.hpp"

#include <unistd.h>

#include <algorithm>
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
#include <variant>
#include <vector>

#include "utils.hpp"

namespace analyser::metric::metric_impl {

namespace rv = std::ranges::views;
namespace rs = std::ranges;

MetricResult::ValueType CodeLinesCountMetric::CalculateImpl(const function::Function &f) const {
    const std::string &ast = f.ast;

    // Находим координаты начала и конца функции
    size_t func_start = ast.find("(function_definition");
    if (func_start == std::string::npos) {
        return 0;
    }

    // Функция для извлечения номера строки из координат
    auto extract_line_number = [](const std::string &text, size_t start_pos) -> std::optional<int> {
        size_t coord_start = text.find('[', start_pos);
        if (coord_start == std::string::npos) {
            return std::nullopt;
        }

        size_t coord_end = text.find(']', coord_start);
        if (coord_end == std::string::npos) {
            return std::nullopt;
        }

        std::string coords = text.substr(coord_start + 1, coord_end - coord_start - 1);
        size_t comma = coords.find(',');
        if (comma == std::string::npos) {
            return std::nullopt;
        }

        try {
            return ToInt(coords.substr(0, comma));
        } catch (...) {
            return std::nullopt;
        }
    };

    // Извлекаем номер начальной строки
    auto start_line_opt = extract_line_number(ast, func_start);
    if (!start_line_opt) {
        return 0;
    }
    int start_line = *start_line_opt;

    // Находим позицию тире для поиска конечных координат
    size_t coord_start = ast.find('[', func_start);
    if (coord_start == std::string::npos) {
        return 0;
    }

    size_t coord_end = ast.find(']', coord_start);
    if (coord_end == std::string::npos) {
        return 0;
    }

    size_t dash = ast.find('-', coord_end);
    if (dash == std::string::npos) {
        return 0;
    }

    // Извлекаем номер конечной строки
    auto end_line_opt = extract_line_number(ast, dash);
    if (!end_line_opt) {
        return 0;
    }
    int end_line = *end_line_opt;

    // Количество строк = разность номеров строк + 1
    return end_line - start_line + 1;
}

std::string CodeLinesCountMetric::Name() const { return "CodeLinesCount"; }

}  // namespace analyser::metric::metric_impl
