#include "Spire/UiViewer/UiProperty.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

const QString& UiProperty::get_name() const {
  return m_name;
}

void UiProperty::disconnect() {
  m_changed_signal.disconnect_all_slots();
}

void UiProperty::reset() {
  m_changed_signal.disconnect_all_slots();
}

connection UiProperty::connect_changed_signal(
    const ChangedSignal::slot_type& slot) const {
  slot(get_value());
  return m_changed_signal.connect(slot);
}

UiProperty::UiProperty(QString name)
  : m_name(std::move(name)) {}

void UiProperty::signal_change() {
  auto value = get_value();
  m_changed_signal(value);
}
