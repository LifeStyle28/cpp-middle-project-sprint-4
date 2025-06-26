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

namespace analyser {

namespace rv = std::ranges::views;
namespace rs = std::ranges;

using Groupped =
    std::unordered_map<std::string,
                       std::vector<std::pair<analyser::function::Function, analyser::metric::MetricResults>>>;

inline auto AnalyseFunctions(const std::vector<std::string> &files,
                             const analyser::metric::MetricExtractor &metric_extractor) {
    auto file_objects =
        files | rv::transform([](const std::string &filename) { return analyser::file::File(filename); });

    auto all_functions = file_objects | rv::transform([](const analyser::file::File &file) {
                             return analyser::function::FunctionExtractor().Get(file);
                         }) |
                         rv::join;

    auto metrics = all_functions | rv::transform([&metric_extractor](const analyser::function::Function &func) {
                       auto result = metric_extractor.Get(func);
                       return std::make_pair(func, result);
                   });

    std::vector<std::pair<analyser::function::Function, analyser::metric::MetricResults>> result;
    for (const auto &pair : metrics) {
        result.push_back(pair);
    }
    return result;
}

inline auto SplitByClasses(const auto &analysis) {
    auto class_methods = analysis | rv::filter([](const auto &pair) {
                             const auto &func = pair.first;
                             return func.class_name.has_value() && !func.class_name->empty();
                         });

    Groupped grouped;
    for (const auto &pair : class_methods) {
        const auto &func = pair.first;
        const auto &class_name = func.class_name.value();
        grouped[class_name].push_back(pair);
    }

    std::vector<std::vector<std::pair<analyser::function::Function, analyser::metric::MetricResults>>> result;
    for (const auto &[_, methods] : grouped) {
        result.push_back(methods);
    }

    return result;
}

inline auto SplitByFiles(const auto &analysis) {
    Groupped grouped;
    for (const auto &pair : analysis) {
        const auto &func = pair.first;
        const auto &filename = func.filename;
        grouped[filename].push_back(pair);
    }

    std::vector<std::vector<std::pair<analyser::function::Function, analyser::metric::MetricResults>>> result;
    for (const auto &[_, functions] : grouped) {
        result.push_back(functions);
    }

    return result;
}

inline void AccumulateFunctionAnalysis(const auto &analysis,
                                       const analyser::metric_accumulator::MetricsAccumulator &accumulator) {
    for (const auto &[_, metric_results] : analysis) {
        accumulator.AccumulateNextFunctionResults(metric_results);
    }
}

}  // namespace analyser
