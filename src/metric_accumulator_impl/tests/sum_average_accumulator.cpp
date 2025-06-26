#include "metric_accumulator_impl/sum_average_accumulator.hpp"

#include <gtest/gtest.h>

#include <cmath>

namespace analyser::metric_accumulator::metric_accumulator_impl::test {

class SumAverageAccumulatorTest : public ::testing::Test {
protected:
    void SetUp() override { accumulator = std::make_unique<SumAverageAccumulator>(); }

    metric::MetricResult CreateMetricResult(int value, const std::string &name = "TestMetric") { return {name, value}; }

    std::unique_ptr<SumAverageAccumulator> accumulator;
};

TEST_F(SumAverageAccumulatorTest, InitialState) {
    auto result = accumulator->Get();
    EXPECT_EQ(result.sum, 0);
    EXPECT_EQ(result.average, 0.0);
}

TEST_F(SumAverageAccumulatorTest, SingleValue) {
    accumulator->Accumulate(CreateMetricResult(10));
    accumulator->Finalize();
    auto result = accumulator->Get();
    EXPECT_EQ(result.sum, 10);
    EXPECT_EQ(result.average, 10.0);
}

TEST_F(SumAverageAccumulatorTest, MultipleValues) {
    accumulator->Accumulate(CreateMetricResult(10));
    accumulator->Accumulate(CreateMetricResult(20));
    accumulator->Accumulate(CreateMetricResult(30));
    accumulator->Finalize();
    auto result = accumulator->Get();
    EXPECT_EQ(result.sum, 60);
    EXPECT_EQ(result.average, 20.0);
}

TEST_F(SumAverageAccumulatorTest, ZeroValues) {
    accumulator->Accumulate(CreateMetricResult(0));
    accumulator->Accumulate(CreateMetricResult(0));
    accumulator->Finalize();
    auto result = accumulator->Get();
    EXPECT_EQ(result.sum, 0);
    EXPECT_EQ(result.average, 0.0);
}

TEST_F(SumAverageAccumulatorTest, NegativeValues) {
    accumulator->Accumulate(CreateMetricResult(-5));
    accumulator->Accumulate(CreateMetricResult(5));
    accumulator->Finalize();
    auto result = accumulator->Get();
    EXPECT_EQ(result.sum, 0);
    EXPECT_EQ(result.average, 0.0);
}

TEST_F(SumAverageAccumulatorTest, NoAccumulationBeforeFinalize) {
    accumulator->Accumulate(CreateMetricResult(10));
    accumulator->Accumulate(CreateMetricResult(20));
    auto result = accumulator->Get();
    EXPECT_EQ(result.sum, 0);
    EXPECT_EQ(result.average, 0.0);
}

TEST_F(SumAverageAccumulatorTest, NoAccumulationAfterFinalize) {
    accumulator->Accumulate(CreateMetricResult(10));
    accumulator->Finalize();
    accumulator->Accumulate(CreateMetricResult(20));
    auto result = accumulator->Get();
    EXPECT_EQ(result.sum, 10);
    EXPECT_EQ(result.average, 10.0);
}

TEST_F(SumAverageAccumulatorTest, Reset) {
    accumulator->Accumulate(CreateMetricResult(10));
    accumulator->Accumulate(CreateMetricResult(20));
    accumulator->Finalize();
    auto result = accumulator->Get();
    EXPECT_EQ(result.sum, 30);
    EXPECT_EQ(result.average, 15.0);

    accumulator->Reset();
    result = accumulator->Get();
    EXPECT_EQ(result.sum, 0);
    EXPECT_EQ(result.average, 0.0);

    accumulator->Accumulate(CreateMetricResult(5));
    accumulator->Finalize();
    result = accumulator->Get();
    EXPECT_EQ(result.sum, 5);
    EXPECT_EQ(result.average, 5.0);
}

TEST_F(SumAverageAccumulatorTest, EmptyAccumulator) {
    accumulator->Finalize();
    auto result = accumulator->Get();
    EXPECT_EQ(result.sum, 0);
    EXPECT_EQ(result.average, 0.0);
}

TEST_F(SumAverageAccumulatorTest, LargeNumbers) {
    accumulator->Accumulate(CreateMetricResult(1000000));
    accumulator->Accumulate(CreateMetricResult(2000000));
    accumulator->Finalize();
    auto result = accumulator->Get();
    EXPECT_EQ(result.sum, 3000000);
    EXPECT_EQ(result.average, 1500000.0);
}

TEST_F(SumAverageAccumulatorTest, SumAverageStructComparison) {
    SumAverageAccumulator::SumAverage sa1{10, 5.0};
    SumAverageAccumulator::SumAverage sa2{10, 5.0};
    SumAverageAccumulator::SumAverage sa3{20, 10.0};

    EXPECT_EQ(sa1, sa2);
    EXPECT_NE(sa1, sa3);
    EXPECT_LT(sa1, sa3);
    EXPECT_GT(sa3, sa1);
}

}  // namespace analyser::metric_accumulator::metric_accumulator_impl::test
