#include "metric_impl/cyclomatic_complexity.hpp"

#include <gtest/gtest.h>

#include "file.hpp"
#include "function.hpp"

namespace analyser::metric::metric_impl {

class CyclomaticComplexityTest : public ::testing::Test {
protected:
    void SetUp() override { metric = std::make_unique<CyclomaticComplexityMetric>(); }

    function::Function CreateFunctionFromFile(const std::string &filename) {
        analyser::file::File file(filename);
        analyser::function::FunctionExtractor extractor;
        auto functions = extractor.Get(file);
        if (functions.empty()) {
            throw std::runtime_error("No functions found in file: " + filename);
        }
        return functions[0];
    }

    int CalculateComplexity(const function::Function &func) { return metric->Calculate(func).value; }

    std::unique_ptr<CyclomaticComplexityMetric> metric;
};

TEST_F(CyclomaticComplexityTest, SimpleFunction) {
    auto func = CreateFunctionFromFile("simple.py");
    EXPECT_EQ(CalculateComplexity(func), 2);  // базовая + assert
}

TEST_F(CyclomaticComplexityTest, IfFunction) {
    auto func = CreateFunctionFromFile("if.py");
    EXPECT_EQ(CalculateComplexity(func), 2);  // базовая + if
}

TEST_F(CyclomaticComplexityTest, LoopsFunction) {
    auto func = CreateFunctionFromFile("loops.py");
    EXPECT_EQ(CalculateComplexity(func), 4);  // базовая + if + while + for
}

TEST_F(CyclomaticComplexityTest, ParametersFunction) {
    auto func = CreateFunctionFromFile("many_parameters.py");
    EXPECT_EQ(CalculateComplexity(func), 2);  // базовая + assert
}

TEST_F(CyclomaticComplexityTest, ManyLinesFunction) {
    auto func = CreateFunctionFromFile("many_lines.py");
    EXPECT_EQ(CalculateComplexity(func), 2);  // базовая + assert
}

TEST_F(CyclomaticComplexityTest, TernaryFunction) {
    auto func = CreateFunctionFromFile("ternary.py");
    EXPECT_EQ(CalculateComplexity(func), 3);  // базовая + 2 тернарных оператора
}

TEST_F(CyclomaticComplexityTest, ExceptionsFunction) {
    auto func = CreateFunctionFromFile("exceptions.py");
    EXPECT_EQ(CalculateComplexity(func), 5);  // базовая + try + except + finally + assert
}

TEST_F(CyclomaticComplexityTest, NestedIfFunction) {
    auto func = CreateFunctionFromFile("nested_if.py");
    EXPECT_EQ(CalculateComplexity(func), 6);  // базовая + 2 if + else + elif + assert
}

TEST_F(CyclomaticComplexityTest, MatchCaseFunction) {
    auto func = CreateFunctionFromFile("match_case.py");
    EXPECT_EQ(CalculateComplexity(func), 5);  // базовая + match + 3 case
}

TEST_F(CyclomaticComplexityTest, MetricName) { EXPECT_EQ(metric->Name(), "CyclomaticComplexity"); }

}  // namespace analyser::metric::metric_impl
