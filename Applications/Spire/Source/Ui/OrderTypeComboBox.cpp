#include "Spire/Ui/OrderTypeComboBox.hpp"
#include <Beam/Collections/EnumIterator.hpp>
#include <QHBoxLayout>

using namespace Beam;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

OrderTypeComboBox::OrderTypeComboBox(QWidget* parent)
  : OrderTypeComboBox(false, parent) {}

OrderTypeComboBox::OrderTypeComboBox(bool has_cell_style, QWidget* parent)
    : QWidget(parent) {
  auto items = [] {
    auto types = std::vector<QVariant>();
    types.reserve(OrderType::COUNT);
    for(auto type : Beam::MakeRange<OrderType>()) {
      types.push_back(QVariant::fromValue(type));
    }
    return types;
  }();
  m_menu = new StaticDropDownMenu(items, this);
  m_menu->set_cell_style(has_cell_style);
  setFocusProxy(m_menu);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->addWidget(m_menu);
  m_value_connection = m_menu->connect_value_selected_signal(
    [=] (const auto& value) {
      m_selected_signal(value.value<OrderType>());
    });
}

connection OrderTypeComboBox::connect_selected_signal(
    const SelectedSignal::slot_type& slot) const {
  return m_selected_signal.connect(slot);
}

OrderType OrderTypeComboBox::get_order_type() const {
  return m_menu->get_current_item().value<OrderType>();
}

void OrderTypeComboBox::set_order_type(Nexus::OrderType type) {
  m_menu->set_current_item(QVariant::fromValue(type));
}
