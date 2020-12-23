#include "Spire/UiViewer/UiProperty.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

const QString& UiProperty::get_name() const {
  return m_name;
}

connection UiProperty::connect_changed_signal(
    const ChangedSignal::slot_type& slot) const {
  return m_changed_signal.connect(slot);
}

UiProperty::UiProperty(QString name)
  : m_name(std::move(name)) {}
