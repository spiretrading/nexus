#include "Spire/UiViewer/UiProfile.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

UiProfile::UiProfile(QString name,
    std::vector<std::shared_ptr<UiProperty>> properties,
    std::function<QWidget* (UiProfile&)> factory)
  : m_event_signal(std::make_shared<EventSignal>()),
    m_name(std::move(name)),
    m_properties(std::move(properties)),
    m_factory(std::move(factory)),
    m_widget(nullptr) {}

const QString& UiProfile::get_name() const {
  return m_name;
}

const std::vector<std::shared_ptr<UiProperty>>&
    UiProfile::get_properties() const {
  return m_properties;
}

QWidget* UiProfile::get_widget() {
  if(!m_widget) {
    m_widget = m_factory(*this);
  }
  return m_widget;
}

void UiProfile::remove_widget() {
  for(auto& property : m_properties) {
    property->disconnect();
  }
  m_widget = nullptr;
}

void UiProfile::reset() {
  m_event_signal->disconnect_all_slots();
  for(auto& property : m_properties) {
    property->reset();
  }
  m_widget = nullptr;
}

connection UiProfile::connect_event_signal(
    const EventSignal::slot_type& slot) const {
  return m_event_signal->connect(slot);
}
