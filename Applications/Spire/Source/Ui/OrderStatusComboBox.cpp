#include "Spire/Ui/OrderStatusComboBox.hpp"
#include <Beam/Collections/EnumIterator.hpp>
#include <QHBoxLayout>

using namespace Beam;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

OrderStatusComboBox::OrderStatusComboBox(QWidget* parent)
    : QWidget(parent) {
  auto items = [] {
    auto statuses = std::vector<QVariant>();
    statuses.reserve(OrderStatus::COUNT);
    for(auto status : Beam::MakeRange<OrderStatus>()) {
      statuses.push_back(QVariant::fromValue(status));
    }
    return statuses;
  }();
  m_menu = new StaticDropDownMenu(items, this);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->addWidget(m_menu);
  m_value_connection = m_menu->connect_value_selected_signal(
    [=] (const auto& value) {
      m_selected_signal(value.value<OrderStatus>());
    });
}

OrderStatus OrderStatusComboBox::get_order_status() const {
  return m_menu->get_current_item().value<OrderStatus>();
}

void OrderStatusComboBox::set_order_status(OrderStatus status) {
  m_menu->set_current_item(QVariant::fromValue(status));
}

connection OrderStatusComboBox::connect_selected_signal(
    const SelectedSignal::slot_type& slot) const {
  return m_selected_signal.connect(slot);
}
