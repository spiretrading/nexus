#include "Spire/TimeAndSales/TimeAndSalesPropertiesWindowFactory.hpp"

using namespace Spire;

TimeAndSalesPropertiesWindowFactory::TimeAndSalesPropertiesWindowFactory()
  : TimeAndSalesPropertiesWindowFactory(
      std::make_shared<LocalTimeAndSalesPropertiesModel>(
        TimeAndSalesProperties::get_default())) {}

TimeAndSalesPropertiesWindowFactory::TimeAndSalesPropertiesWindowFactory(
  std::shared_ptr<TimeAndSalesPropertiesModel> properties)
  : m_properties(std::move(properties)) {}

const std::shared_ptr<TimeAndSalesPropertiesModel>&
    TimeAndSalesPropertiesWindowFactory::get_properties() const {
  return m_properties;
}

TimeAndSalesPropertiesWindow* TimeAndSalesPropertiesWindowFactory::make(
    std::shared_ptr<ProxyValueModel<TimeAndSalesProperties>> live_preview) {
  if(m_live_preview == live_preview && m_properties_window) {
    return m_properties_window.get();
  }
  if(m_live_preview) {
    m_live_preview->set_source(m_properties);
  }
  m_preview = std::make_shared<LocalTimeAndSalesPropertiesModel>(
    m_properties->get());
  m_live_preview = std::move(live_preview);
  m_live_preview->set_source(m_preview);
  m_properties_window =
    std::make_unique<TimeAndSalesPropertiesWindow>(m_preview);
  m_commit_connection = m_properties_window->connect_commit_signal(
    std::bind_front(&TimeAndSalesPropertiesWindowFactory::on_commit, this));
  m_cancel_connection = m_properties_window->connect_cancel_signal(
    std::bind_front(&TimeAndSalesPropertiesWindowFactory::on_cancel, this));
  return m_properties_window.get();
}

void TimeAndSalesPropertiesWindowFactory::on_commit() {
  if(!m_live_preview) {
    return;
  }
  m_properties->set(m_preview->get());
  m_live_preview->set_source(m_properties);
  m_live_preview.reset();
  m_preview.reset();
}

void TimeAndSalesPropertiesWindowFactory::on_cancel() {
  if(!m_live_preview) {
    return;
  }
  m_live_preview->set_source(m_properties);
  m_live_preview.reset();
  m_preview.reset();
}
