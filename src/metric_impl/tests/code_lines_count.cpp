#include "metric_impl/code_lines_count.hpp"

#include <gtest/gtest.h>

#include "file.hpp"
#include "function.hpp"

namespace analyser::metric::metric_impl {

class CodeLinesCountTest : public ::testing::Test {
protected:
    void SetUp() override { metric = std::make_unique<CodeLinesCountMetric>(); }

    function::Function CreateFunctionFromFile(const std::string &filename) {
        analyser::file::File file(filename);
        analyser::function::FunctionExtractor extractor;
        auto functions = extractor.Get(file);
        if (functions.empty()) {
            throw std::runtime_error("No functions found in file: " + filename);
        }
        return functions[0];
    }

    int CountLines(const function::Function &func) { return metric->Calculate(func).value; }

    std::unique_ptr<CodeLinesCountMetric> metric;
};

TEST_F(CodeLinesCountTest, SimpleFunction) {
    auto func = CreateFunctionFromFile("simple.py");
    EXPECT_EQ(CountLines(func), 7);
}

TEST_F(CodeLinesCountTest, IfFunction) {
    auto func = CreateFunctionFromFile("if.py");
    EXPECT_EQ(CountLines(func), 4);
}

TEST_F(CodeLinesCountTest, LoopsFunction) {
    auto func = CreateFunctionFromFile("loops.py");
    EXPECT_EQ(CountLines(func), 7);
}

TEST_F(CodeLinesCountTest, ManyLinesFunction) {
    auto func = CreateFunctionFromFile("many_lines.py");
    EXPECT_EQ(CountLines(func), 15);
}

TEST_F(CodeLinesCountTest, ParametersFunction) {
    auto func = CreateFunctionFromFile("many_parameters.py");
    EXPECT_EQ(CountLines(func), 2);
}

TEST_F(CodeLinesCountTest, MetricName) { EXPECT_EQ(metric->Name(), "CodeLinesCount"); }

}  // namespace analyser::metric::metric_impl
