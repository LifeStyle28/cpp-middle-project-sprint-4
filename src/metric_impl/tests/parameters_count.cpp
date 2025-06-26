#include "metric_impl/parameters_count.hpp"

#include <gtest/gtest.h>

#include "file.hpp"
#include "function.hpp"

namespace analyser::metric::metric_impl {

class ParametersCountTest : public ::testing::Test {
protected:
    void SetUp() override { metric = std::make_unique<CountParametersMetric>(); }

    function::Function CreateFunctionFromFile(const std::string &filename) {
        analyser::file::File file(filename);
        analyser::function::FunctionExtractor extractor;
        auto functions = extractor.Get(file);

        if (functions.empty()) {
            throw std::runtime_error("No functions found in file: " + filename);
        }
        return functions[0];
    }

    int CountParameters(const function::Function &func) { return metric->Calculate(func).value; }

    std::unique_ptr<CountParametersMetric> metric;
};

TEST_F(ParametersCountTest, NoParameters) {
    auto func = CreateFunctionFromFile("simple.py");
    EXPECT_EQ(CountParameters(func), 0);
}

TEST_F(ParametersCountTest, MultipleParameters) {
    auto func = CreateFunctionFromFile("many_parameters.py");
    EXPECT_EQ(CountParameters(func), 5);  // a, b, c=5, *args, **kwargs
}

TEST_F(ParametersCountTest, SingleParameter) {
    auto func = CreateFunctionFromFile("if.py");
    EXPECT_EQ(CountParameters(func), 1);  // x
}

TEST_F(ParametersCountTest, SingleParameterLoops) {
    auto func = CreateFunctionFromFile("loops.py");
    EXPECT_EQ(CountParameters(func), 1);  // n
}

TEST_F(ParametersCountTest, NoParametersManyLines) {
    auto func = CreateFunctionFromFile("many_lines.py");
    EXPECT_EQ(CountParameters(func), 0);
}

TEST_F(ParametersCountTest, MetricName) { EXPECT_EQ(metric->Name(), "ParametersCount"); }

}  // namespace analyser::metric::metric_impl
