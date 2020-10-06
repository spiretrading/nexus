#include "Spire/Ui/OrderTypeComboBox.hpp"
#include <QHBoxLayout>
#include "Spire/Ui/StaticDropDownMenu.hpp"

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

OrderTypeComboBox::OrderTypeComboBox(QWidget* parent)
    : QWidget(parent) {
  auto menu = new StaticDropDownMenu(make_order_type_list(), this);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->addWidget(menu);
  m_value_connection = menu->connect_value_selected_signal(
    [=] (const auto& value) { m_selected_signal(value.value<OrderType>()); });
}

connection OrderTypeComboBox::connect_selected_signal(
    const SelectedSignal::slot_type& slot) const {
  return m_selected_signal.connect(slot);
}
