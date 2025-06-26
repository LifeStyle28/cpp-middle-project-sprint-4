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

#include "metric.hpp"

namespace analyser::metric::metric_impl {

struct CountParametersMetric final : public IMetric {
    MetricResult::ValueType CalculateImpl(const function::Function &f) const override;
    std::string Name() const override;
};

}  // namespace analyser::metric::metric_impl
