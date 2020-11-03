#include "Spire/Ui/OrderTypeComboBox.hpp"
#include <Beam/Collections/EnumIterator.hpp>

using namespace Beam;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

OrderTypeComboBox::OrderTypeComboBox(QWidget* parent)
    : StaticDropDownMenu({}, parent) {
  auto items = [] {
    auto types = std::vector<QVariant>();
    types.reserve(OrderType::COUNT);
    for(auto type : Beam::MakeRange<OrderType>()) {
      types.push_back(QVariant::fromValue(type));
    }
    return types;
  }();
  set_items(items);
  m_value_connection = connect_value_selected_signal(
    [=] (const auto& value) {
      m_selected_signal(value.value<OrderType>());
    });
}

connection OrderTypeComboBox::connect_selected_signal(
    const SelectedSignal::slot_type& slot) const {
  return m_selected_signal.connect(slot);
}

OrderType OrderTypeComboBox::get_current_order_type() const {
  return get_current_item().value<OrderType>();
}

OrderType OrderTypeComboBox::get_last_order_type() const {
  return get_last_item().value<OrderType>();
}

void OrderTypeComboBox::set_current_order_type(Nexus::OrderType type) {
  set_current_item(QVariant::fromValue(type));
}
