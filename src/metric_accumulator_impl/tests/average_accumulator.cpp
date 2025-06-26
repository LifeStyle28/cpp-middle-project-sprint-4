#include "metric_accumulator_impl/average_accumulator.hpp"

#include <gtest/gtest.h>

#include <cmath>

namespace analyser::metric_accumulator::metric_accumulator_impl::test {

class AverageAccumulatorTest : public ::testing::Test {
protected:
    void SetUp() override { accumulator = std::make_unique<AverageAccumulator>(); }

    metric::MetricResult CreateMetricResult(int value, const std::string &name = "TestMetric") { return {name, value}; }

    std::unique_ptr<AverageAccumulator> accumulator;
};

TEST_F(AverageAccumulatorTest, InitialState) { EXPECT_EQ(accumulator->Get(), 0.0); }

TEST_F(AverageAccumulatorTest, SingleValue) {
    accumulator->Accumulate(CreateMetricResult(10));
    accumulator->Finalize();
    EXPECT_EQ(accumulator->Get(), 10.0);
}

TEST_F(AverageAccumulatorTest, MultipleValues) {
    accumulator->Accumulate(CreateMetricResult(10));
    accumulator->Accumulate(CreateMetricResult(20));
    accumulator->Accumulate(CreateMetricResult(30));
    accumulator->Finalize();
    EXPECT_EQ(accumulator->Get(), 20.0);
}

TEST_F(AverageAccumulatorTest, ZeroValues) {
    accumulator->Accumulate(CreateMetricResult(0));
    accumulator->Accumulate(CreateMetricResult(0));
    accumulator->Finalize();
    EXPECT_EQ(accumulator->Get(), 0.0);
}

TEST_F(AverageAccumulatorTest, NegativeValues) {
    accumulator->Accumulate(CreateMetricResult(-5));
    accumulator->Accumulate(CreateMetricResult(5));
    accumulator->Finalize();
    EXPECT_EQ(accumulator->Get(), 0.0);
}

TEST_F(AverageAccumulatorTest, NoAccumulationBeforeFinalize) {
    accumulator->Accumulate(CreateMetricResult(10));
    accumulator->Accumulate(CreateMetricResult(20));
    EXPECT_EQ(accumulator->Get(), 0.0);
}

TEST_F(AverageAccumulatorTest, NoAccumulationAfterFinalize) {
    accumulator->Accumulate(CreateMetricResult(10));
    accumulator->Finalize();
    accumulator->Accumulate(CreateMetricResult(20));
    EXPECT_EQ(accumulator->Get(), 10.0);
}

TEST_F(AverageAccumulatorTest, Reset) {
    accumulator->Accumulate(CreateMetricResult(10));
    accumulator->Accumulate(CreateMetricResult(20));
    accumulator->Finalize();
    EXPECT_EQ(accumulator->Get(), 15.0);

    accumulator->Reset();
    EXPECT_EQ(accumulator->Get(), 0.0);

    accumulator->Accumulate(CreateMetricResult(5));
    accumulator->Finalize();
    EXPECT_EQ(accumulator->Get(), 5.0);
}

TEST_F(AverageAccumulatorTest, EmptyAccumulator) {
    accumulator->Finalize();
    EXPECT_EQ(accumulator->Get(), 0.0);
}

TEST_F(AverageAccumulatorTest, LargeNumbers) {
    accumulator->Accumulate(CreateMetricResult(1000000));
    accumulator->Accumulate(CreateMetricResult(2000000));
    accumulator->Finalize();
    EXPECT_EQ(accumulator->Get(), 1500000.0);
}

}  // namespace analyser::metric_accumulator::metric_accumulator_impl::test
