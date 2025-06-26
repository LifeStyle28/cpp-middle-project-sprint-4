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
#include <stdexcept>

#include "analyse.hpp"
#include "cmd_options.hpp"
#include "file.hpp"
#include "function.hpp"
#include "metric.hpp"
#include "metric_accumulator.hpp"
#include "metric_accumulator_impl/accumulators.hpp"
#include "metric_impl/metrics.hpp"

// Вспомогательные функции для обработки ошибок
void validateInputFiles(const std::vector<std::string>& files) {
    if (files.empty()) {
        throw std::runtime_error("No input files specified. Please provide at least one file to analyze.");
    }

    for (const auto& file : files) {
        if (!std::filesystem::exists(file)) {
            throw std::runtime_error("File does not exist: " + file);
        }
        if (!std::filesystem::is_regular_file(file)) {
            throw std::runtime_error("Path is not a regular file: " + file);
        }
    }
}

void validateAnalysisResults(const auto& analysis_results) {
    if (analysis_results.empty()) {
        throw std::runtime_error("No functions found in the provided files. Please check if the files contain valid C++ code.");
    }
}

template<typename T>
T safeGetAccumulator(const analyser::metric_accumulator::MetricsAccumulator& accumulator,
                     const std::string& metric_name,
                     const std::string& context) {
    try {
        return accumulator.GetFinalizedAccumulator<T>(metric_name);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to get " + metric_name + " accumulator for " + context + ": " + e.what());
    }
}

int main(int argc, char *argv[]) {
    try {
        // распарсите входные параметры
        analyser::cmd::ProgramOptions options;
        if (!options.Parse(argc, argv)) {
            std::print(stderr, "Error: Failed to parse command line arguments.\n");
            return 1;
        }

        const auto& files = options.GetFiles();

        // Валидация входных файлов
        try {
            validateInputFiles(files);
        } catch (const std::runtime_error& e) {
            std::print(stderr, "Input validation error: {}\n", e.what());
            return 1;
        }

        // зарегистрируйте метрики в metric_extractor
        analyser::metric::MetricExtractor metric_extractor;
        try {
            metric_extractor.RegisterMetric(std::make_unique<analyser::metric::metric_impl::CodeLinesCountMetric>());
            metric_extractor.RegisterMetric(std::make_unique<analyser::metric::metric_impl::CyclomaticComplexityMetric>());
            metric_extractor.RegisterMetric(std::make_unique<analyser::metric::metric_impl::CountParametersMetric>());
            // metric_extractor.RegisterMetric(std::make_unique<analyser::metric::metric_impl::NamingStyleMetric>());
        } catch (const std::exception& e) {
            throw std::runtime_error("Failed to register metrics: " + std::string(e.what()));
        }

        // запустите analyser::AnalyseFunctions
        auto analysis_results = analyser::AnalyseFunctions(files, metric_extractor);

        // Валидация результатов анализа
        try {
            validateAnalysisResults(analysis_results);
        } catch (const std::runtime_error& e) {
            std::print(stderr, "Analysis error: {}\n", e.what());
            return 1;
        }

        // зарегистрируйте аккумуляторы метрик в accumulator
        analyser::metric_accumulator::MetricsAccumulator accumulator;
        try {
            accumulator.RegisterAccumulator("CodeLinesCount", std::make_unique<analyser::metric_accumulator::metric_accumulator_impl::AverageAccumulator>());
            accumulator.RegisterAccumulator("CyclomaticComplexity", std::make_unique<analyser::metric_accumulator::metric_accumulator_impl::SumAverageAccumulator>());
            accumulator.RegisterAccumulator("ParametersCount", std::make_unique<analyser::metric_accumulator::metric_accumulator_impl::AverageAccumulator>());
            // accumulator.RegisterAccumulator("NamingStyle", std::make_unique<analyser::metric_accumulator::metric_accumulator_impl::CategoricalAccumulator>());
        } catch (const std::exception& e) {
            throw std::runtime_error("Failed to register metric accumulators: " + std::string(e.what()));
        }

        // выведете результаты анализа на консоль
        std::print("Function Analysis Results:\n");
        std::print("========================\n");
        for (const auto& [func, metrics] : analysis_results) {
            std::string location = func.filename;
            if (func.class_name.has_value() && !func.class_name->empty()) {
                location += "[::" + func.class_name.value() + "]";
            }
            location += "::" + func.name;

            std::print("{}\n", location);
            for (const auto& metric : metrics) {
                std::print("    {}: {}\n", metric.metric_name, metric.value);
            }
            std::print("\n");
        }

        // запустите analyser::SplitByFiles
        std::print("File-based Aggregation:\n");
        std::print("======================\n");
        auto file_groups = analyser::SplitByFiles(analysis_results);
        for (const auto& file_group : file_groups) {
            if (file_group.empty()) continue;

            const std::string& filename = file_group[0].first.filename;
            std::print("Accumulated Analysis for file {}:\n", filename);

            try {
                accumulator.ResetAccumulators();
                // запустите analyser::AccumulateFunctionAnalysis для каждого подмножества результатов метрик
                analyser::AccumulateFunctionAnalysis(file_group, accumulator);

                // выведете результаты на консоль
                try {
                    auto code_lines_acc = safeGetAccumulator<analyser::metric_accumulator::metric_accumulator_impl::AverageAccumulator>(
                        accumulator, "CodeLinesCount", "file " + filename);
                    std::print("    CodeLinesCount: {}\n", code_lines_acc.Get());
                } catch (const std::runtime_error& e) {
                    std::print(stderr, "Warning: {}\n", e.what());
                }

                try {
                    auto complexity_acc = safeGetAccumulator<analyser::metric_accumulator::metric_accumulator_impl::SumAverageAccumulator>(
                        accumulator, "CyclomaticComplexity", "file " + filename);
                    auto complexity_result = complexity_acc.Get();
                    std::print("    CyclomaticComplexity_sum: {}\n", complexity_result.sum);
                    std::print("    CyclomaticComplexity_avg: {}\n", complexity_result.average);
                } catch (const std::runtime_error& e) {
                    std::print(stderr, "Warning: {}\n", e.what());
                }

                try {
                    auto params_acc = safeGetAccumulator<analyser::metric_accumulator::metric_accumulator_impl::AverageAccumulator>(
                        accumulator, "ParametersCount", "file " + filename);
                    std::print("    ParametersCount: {}\n", params_acc.Get());
                } catch (const std::runtime_error& e) {
                    std::print(stderr, "Warning: {}\n", e.what());
                }

            } catch (const std::exception& e) {
                std::print(stderr, "Error processing file {}: {}\n", filename, e.what());
            }

            std::print("\n");
        }

        // запустите analyser::SplitByClasses
        std::print("Class-based Aggregation:\n");
        std::print("========================\n");
        auto class_groups = analyser::SplitByClasses(analysis_results);
        for (const auto& class_group : class_groups) {
            if (class_group.empty()) continue;

            const std::string& class_name = class_group[0].first.class_name.value();
            std::print("Accumulated Analysis for class {}:\n", class_name);

            try {
                accumulator.ResetAccumulators();
                // запустите analyser::AccumulateFunctionAnalysis для каждого подмножества результатов метрик
                analyser::AccumulateFunctionAnalysis(class_group, accumulator);

                // выведете результаты на консоль
                try {
                    auto code_lines_acc = safeGetAccumulator<analyser::metric_accumulator::metric_accumulator_impl::AverageAccumulator>(
                        accumulator, "CodeLinesCount", "class " + class_name);
                    std::print("    CodeLinesCount: {}\n", code_lines_acc.Get());
                } catch (const std::runtime_error& e) {
                    std::print(stderr, "Warning: {}\n", e.what());
                }

                try {
                    auto complexity_acc = safeGetAccumulator<analyser::metric_accumulator::metric_accumulator_impl::SumAverageAccumulator>(
                        accumulator, "CyclomaticComplexity", "class " + class_name);
                    auto complexity_result = complexity_acc.Get();
                    std::print("    CyclomaticComplexity_sum: {}\n", complexity_result.sum);
                    std::print("    CyclomaticComplexity_avg: {}\n", complexity_result.average);
                } catch (const std::runtime_error& e) {
                    std::print(stderr, "Warning: {}\n", e.what());
                }

                try {
                    auto params_acc = safeGetAccumulator<analyser::metric_accumulator::metric_accumulator_impl::AverageAccumulator>(
                        accumulator, "ParametersCount", "class " + class_name);
                    std::print("    ParametersCount: {}\n", params_acc.Get());
                } catch (const std::runtime_error& e) {
                    std::print(stderr, "Warning: {}\n", e.what());
                }

            } catch (const std::exception& e) {
                std::print(stderr, "Error processing class {}: {}\n", class_name, e.what());
            }

            std::print("\n");
        }

        // запустите analyser::AccumulateFunctionAnalysis для всех результатов метрик
        try {
            analyser::AccumulateFunctionAnalysis(analysis_results, accumulator);
        } catch (const std::exception& e) {
            throw std::runtime_error("Failed to accumulate overall function analysis: " + std::string(e.what()));
        }

        // выведете общие результаты анализа на консоль
        std::print("Overall Analysis Results:\n");
        std::print("========================\n");
        try {
            auto code_lines_acc = safeGetAccumulator<analyser::metric_accumulator::metric_accumulator_impl::AverageAccumulator>(
                accumulator, "CodeLinesCount", "overall analysis");
            std::print("CodeLinesCount: {}\n", code_lines_acc.Get());
        } catch (const std::runtime_error& e) {
            std::print(stderr, "Warning: {}\n", e.what());
        }

        try {
            auto complexity_acc = safeGetAccumulator<analyser::metric_accumulator::metric_accumulator_impl::SumAverageAccumulator>(
                accumulator, "CyclomaticComplexity", "overall analysis");
            auto complexity_result = complexity_acc.Get();
            std::print("CyclomaticComplexity_sum: {}\n", complexity_result.sum);
            std::print("CyclomaticComplexity_avg: {}\n", complexity_result.average);
        } catch (const std::runtime_error& e) {
            std::print(stderr, "Warning: {}\n", e.what());
        }

        try {
            auto params_acc = safeGetAccumulator<analyser::metric_accumulator::metric_accumulator_impl::AverageAccumulator>(
                accumulator, "ParametersCount", "overall analysis");
            std::print("ParametersCount: {}\n", params_acc.Get());
        } catch (const std::runtime_error& e) {
            std::print(stderr, "Warning: {}\n", e.what());
        }

        std::print("\n");

    } catch (const std::runtime_error& e) {
        std::print(stderr, "Runtime error: {}\n", e.what());
        return 1;
    } catch (const std::exception& e) {
        std::print(stderr, "Unexpected error: {}\n", e.what());
        return 1;
    } catch (...) {
        std::print(stderr, "Unknown error occurred.\n");
        return 1;
    }

    return 0;
}
