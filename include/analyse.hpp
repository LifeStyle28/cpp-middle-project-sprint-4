#include <unistd.h>

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <print>
#include <ranges>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

#include "file.hpp"
#include "function.hpp"
#include "metric.hpp"
#include "metric_accumulator.hpp"

#include <range/v3/view/filter.hpp>
#include <range/v3/view/group_by.hpp>

namespace analyser {

namespace rv = std::ranges::views;
namespace rs = std::ranges;

using Groupped =
    std::unordered_map<std::string,
                       std::vector<std::pair<analyser::function::Function, analyser::metric::MetricResults>>>;

inline auto AnalyseFunctions(const std::vector<std::string> &files,
                             const analyser::metric::MetricExtractor &metric_extractor) {
    auto metrics = files | rv::transform([](const std::string &filename) { return analyser::file::File(filename); }) |
                   rv::transform([](const analyser::file::File &file) {
                       return analyser::function::FunctionExtractor().Get(file);
                   }) |
                   rv::join | rv::transform([&metric_extractor](const analyser::function::Function &func) {
                       auto result = metric_extractor.Get(func);
                       return std::make_pair(func, result);
                   });

    std::vector<std::pair<analyser::function::Function, analyser::metric::MetricResults>> result;
    result = rs::to<decltype(result)>(metrics);
    return result;
}

inline auto SplitByClasses(const auto &analysis) {
    return analysis | ranges::views::filter([](const auto &pair) {
               const auto &func = pair.first;
               return func.class_name.has_value() && !func.class_name->empty();
           }) |
           ranges::views::group_by(
               [](const auto &a, const auto &b) { return a.first.class_name.value() == b.first.class_name.value(); });
}

inline auto SplitByFiles(const auto &analysis) {
    return analysis |
           ranges::views::group_by([](const auto &a, const auto &b) { return a.first.filename == b.first.filename; });
}

inline void AccumulateFunctionAnalysis(const auto &analysis,
                                       const analyser::metric_accumulator::MetricsAccumulator &accumulator) {
    for (const auto &[_, metric_results] : analysis) {
        accumulator.AccumulateNextFunctionResults(metric_results);
    }
}
}  // namespace analyser
