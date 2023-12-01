#include "Spire/TimeAndSales/TimeAndSalesPropertiesWindowFactory.hpp"
#include "Spire/Spire/LocalValueModel.hpp"

using namespace Spire;

TimeAndSalesPropertiesWindowFactory::TimeAndSalesPropertiesWindowFactory()
  : TimeAndSalesPropertiesWindowFactory(
      std::make_shared<LocalValueModel<TimeAndSalesProperties>>()) {}

TimeAndSalesPropertiesWindowFactory::TimeAndSalesPropertiesWindowFactory(
  std::shared_ptr<TimeAndSalesPropertiesModel> properties)
  : m_properties(std::move(properties)) {}

TimeAndSalesPropertiesWindow* TimeAndSalesPropertiesWindowFactory::create() {
  if(m_properties_window) {
    return m_properties_window.get();
  }
  m_properties_window =
    std::make_unique<TimeAndSalesPropertiesWindow>(m_properties);
  return m_properties_window.get();
}

const std::shared_ptr<TimeAndSalesPropertiesModel>&
    TimeAndSalesPropertiesWindowFactory::get_properties() const {
  return m_properties;
}
