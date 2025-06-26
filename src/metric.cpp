#include "metric.hpp"

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

#include "function.hpp"

namespace analyser::metric {

void MetricExtractor::RegisterMetric(std::unique_ptr<IMetric> metric) {
    if (metric) {
        metrics.push_back(std::move(metric));
    }
}

MetricResults MetricExtractor::Get(const function::Function &func) const {
    MetricResults results;
    results.reserve(metrics.size());

    for (const auto &metric : metrics) {
        if (metric) {
            results.push_back(metric->Calculate(func));
        }
    }

    return results;
}

}  // namespace analyser::metric
