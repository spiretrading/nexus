#include "Spire/TimeAndSales/TimeAndSalesPropertiesWindowFactory.hpp"

using namespace Spire;

TimeAndSalesPropertiesWindowFactory::TimeAndSalesPropertiesWindowFactory()
  : TimeAndSalesPropertiesWindowFactory(
      std::make_shared<LocalTimeAndSalesPropertiesModel>()) {}

TimeAndSalesPropertiesWindowFactory::TimeAndSalesPropertiesWindowFactory(
  std::shared_ptr<TimeAndSalesPropertiesModel> properties)
  : m_properties(std::move(properties)) {}

TimeAndSalesPropertiesWindow* TimeAndSalesPropertiesWindowFactory::make() {
  if(m_properties_window) {
    return m_properties_window.get();
  }
  m_properties_window =
    std::make_unique<TimeAndSalesPropertiesWindow>(m_properties);
  return m_properties_window.get();
}
