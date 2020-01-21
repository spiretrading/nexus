#include "Spire/Spire/LocalRangeInputModel.hpp"
#include "Spire/Spire/Utility.hpp"

using namespace Nexus;
using namespace Spire;

LocalRangeInputModel::LocalRangeInputModel(
    const std::vector<ChartValue> data)
    : m_data(data),
      m_minimum_value(0),
      m_maximum_value(std::numeric_limits<int>::max()) {
  if(!m_data.empty()) {
    m_minimum_value = data.front();
    m_maximum_value = data.front();
    for(auto& value : m_data) {
      m_minimum_value = min(value, m_minimum_value);
      m_maximum_value = max(value, m_maximum_value);
    }
  }
}

ChartValue LocalRangeInputModel::get_minimum_value() const {
  return m_minimum_value;
}

ChartValue LocalRangeInputModel::get_maximum_value() const {
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
    auto index =
      static_cast<int>((value - m_minimum_value) /
      (m_maximum_value - m_minimum_value) * bin_count);
    index = min(bin_count - 1, index);
    histogram[index]++;
    highest_frequency = max(histogram[index], highest_frequency);
  }
  return {histogram, highest_frequency};
}
