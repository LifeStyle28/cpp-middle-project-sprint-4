#include "metric_accumulator_impl/categorical_accumulator.hpp"

#include <gtest/gtest.h>

#include <stdexcept>

namespace analyser::metric_accumulator::metric_accumulator_impl::test {

class CategoricalAccumulatorTest : public ::testing::Test {
protected:
    void SetUp() override { accumulator = std::make_unique<CategoricalAccumulator>(); }

    metric::MetricResult CreateMetricResult(int value, const std::string &name = "TestMetric") { return {name, value}; }

    std::unique_ptr<CategoricalAccumulator> accumulator;
};

TEST_F(CategoricalAccumulatorTest, InitialState) {
    auto result = accumulator->Get();
    EXPECT_TRUE(result.empty());
}

TEST_F(CategoricalAccumulatorTest, SingleCategory) {
    accumulator->Accumulate(CreateMetricResult(5, "CyclomaticComplexity"));
    accumulator->Finalize();
    auto result = accumulator->Get();
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result["CyclomaticComplexity"], 5);
}

TEST_F(CategoricalAccumulatorTest, MultipleCategories) {
    accumulator->Accumulate(CreateMetricResult(5, "CyclomaticComplexity"));
    accumulator->Accumulate(CreateMetricResult(3, "ParametersCount"));
    accumulator->Accumulate(CreateMetricResult(10, "CodeLinesCount"));
    accumulator->Finalize();
    auto result = accumulator->Get();
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result["CyclomaticComplexity"], 5);
    EXPECT_EQ(result["ParametersCount"], 3);
    EXPECT_EQ(result["CodeLinesCount"], 10);
}

TEST_F(CategoricalAccumulatorTest, SameCategoryMultipleTimes) {
    accumulator->Accumulate(CreateMetricResult(5, "CyclomaticComplexity"));
    accumulator->Accumulate(CreateMetricResult(3, "CyclomaticComplexity"));
    accumulator->Accumulate(CreateMetricResult(7, "CyclomaticComplexity"));
    accumulator->Finalize();
    auto result = accumulator->Get();
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result["CyclomaticComplexity"], 15);
}

TEST_F(CategoricalAccumulatorTest, ZeroValues) {
    accumulator->Accumulate(CreateMetricResult(0, "CyclomaticComplexity"));
    accumulator->Accumulate(CreateMetricResult(0, "ParametersCount"));
    accumulator->Finalize();
    auto result = accumulator->Get();
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result["CyclomaticComplexity"], 0);
    EXPECT_EQ(result["ParametersCount"], 0);
}

TEST_F(CategoricalAccumulatorTest, NegativeValues) {
    accumulator->Accumulate(CreateMetricResult(-5, "CyclomaticComplexity"));
    accumulator->Accumulate(CreateMetricResult(10, "ParametersCount"));
    accumulator->Finalize();
    auto result = accumulator->Get();
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result["CyclomaticComplexity"], -5);
    EXPECT_EQ(result["ParametersCount"], 10);
}

TEST_F(CategoricalAccumulatorTest, NoAccumulationAfterFinalize) {
    accumulator->Accumulate(CreateMetricResult(5, "CyclomaticComplexity"));
    accumulator->Finalize();
    accumulator->Accumulate(CreateMetricResult(3, "ParametersCount"));
    auto result = accumulator->Get();
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result["CyclomaticComplexity"], 5);
    EXPECT_EQ(result.find("ParametersCount"), result.end());
}

TEST_F(CategoricalAccumulatorTest, Reset) {
    accumulator->Accumulate(CreateMetricResult(5, "CyclomaticComplexity"));
    accumulator->Accumulate(CreateMetricResult(3, "ParametersCount"));
    accumulator->Finalize();
    auto result = accumulator->Get();
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result["CyclomaticComplexity"], 5);
    EXPECT_EQ(result["ParametersCount"], 3);

    accumulator->Reset();
    result = accumulator->Get();
    EXPECT_TRUE(result.empty());

    accumulator->Accumulate(CreateMetricResult(10, "CodeLinesCount"));
    accumulator->Finalize();
    result = accumulator->Get();
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result["CodeLinesCount"], 10);
}

TEST_F(CategoricalAccumulatorTest, EmptyAccumulator) {
    accumulator->Finalize();
    auto result = accumulator->Get();
    EXPECT_TRUE(result.empty());
}

TEST_F(CategoricalAccumulatorTest, LargeNumbers) {
    accumulator->Accumulate(CreateMetricResult(1000000, "CyclomaticComplexity"));
    accumulator->Accumulate(CreateMetricResult(2000000, "ParametersCount"));
    accumulator->Finalize();
    auto result = accumulator->Get();
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result["CyclomaticComplexity"], 1000000);
    EXPECT_EQ(result["ParametersCount"], 2000000);
}

TEST_F(CategoricalAccumulatorTest, ComplexCategoryNames) {
    accumulator->Accumulate(CreateMetricResult(5, "Cyclomatic_Complexity_Metric"));
    accumulator->Accumulate(CreateMetricResult(3, "Parameters_Count_Metric"));
    accumulator->Accumulate(CreateMetricResult(10, "Code_Lines_Count_Metric"));
    accumulator->Finalize();
    auto result = accumulator->Get();
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result["Cyclomatic_Complexity_Metric"], 5);
    EXPECT_EQ(result["Parameters_Count_Metric"], 3);
    EXPECT_EQ(result["Code_Lines_Count_Metric"], 10);
}

TEST_F(CategoricalAccumulatorTest, MixedAccumulation) {
    // Смешиваем накопление до и после финализации
    accumulator->Accumulate(CreateMetricResult(5, "Metric1"));
    accumulator->Accumulate(CreateMetricResult(3, "Metric2"));
    accumulator->Finalize();

    // Попытка накопления после финализации
    accumulator->Accumulate(CreateMetricResult(7, "Metric3"));
    accumulator->Accumulate(CreateMetricResult(2, "Metric1"));

    auto result = accumulator->Get();
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result["Metric1"], 5);
    EXPECT_EQ(result["Metric2"], 3);
    EXPECT_EQ(result.find("Metric3"), result.end());
}

}  // namespace analyser::metric_accumulator::metric_accumulator_impl::test
