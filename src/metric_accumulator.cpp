#include "metric_accumulator.hpp"

#include <unistd.h>

#include <algorithm>
#include <any>
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

namespace analyser::metric_accumulator {

void MetricsAccumulator::AccumulateNextFunctionResults(const std::vector<metric::MetricResult> &metric_results) const {
    for (const auto &metric_result : metric_results) {
        auto it = accumulators.find(metric_result.metric_name);
        if (it != accumulators.end() && it->second) {
            it->second->Accumulate(metric_result);
        } else [[unlikely]] {
            throw std::runtime_error("Accumulator not found or null for metric: " + metric_result.metric_name);
        }
    }
}

void MetricsAccumulator::ResetAccumulators() {
    for (auto &[_, accumulator] : accumulators) {
        if (accumulator) {
            accumulator->Reset();
        }
    }
}

}  // namespace analyser::metric_accumulator
