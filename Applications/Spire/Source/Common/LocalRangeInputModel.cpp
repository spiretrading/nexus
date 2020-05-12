#include "Spire/Spire/LocalRangeInputModel.hpp"
#include <algorithm>

using namespace Nexus;
using namespace Spire;

LocalRangeInputModel::LocalRangeInputModel(std::vector<Scalar> data)
    : m_data(std::move(data)) {
  if(m_data.empty()) {
    m_minimum_value = Scalar(Quantity(0));
    m_maximum_value = Scalar(Quantity(0));
  } else {
    m_minimum_value = m_data.front();
    m_maximum_value = m_data.front();
    for(auto& value : m_data) {
      m_minimum_value = std::min(value, m_minimum_value);
      m_maximum_value = std::max(value, m_maximum_value);
    }
  }
}

Scalar LocalRangeInputModel::get_minimum_value() const {
  return m_minimum_value;
}

Scalar LocalRangeInputModel::get_maximum_value() const {
  return m_maximum_value;
}

RangeInputModel::Histogram
    LocalRangeInputModel::make_histogram(int bin_count) const {
  if(bin_count < 1) {
    return {{}, 0};
  }
  auto histogram = std::vector<int>(bin_count, 0);
  auto highest_frequency = 0;
  for(auto& value : m_data) {
    auto index = static_cast<int>((value - m_minimum_value) /
      (m_maximum_value - m_minimum_value) * bin_count);
    index = std::min(bin_count - 1, index);
    histogram[index]++;
    highest_frequency = std::max(histogram[index], highest_frequency);
  }
  return {histogram, highest_frequency};
}
