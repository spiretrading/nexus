#include "Spire/Ui/OrderStatusComboBox.hpp"
#include <QHBoxLayout>
#include "Spire/Ui/StaticDropDownMenu.hpp"

using namespace Beam;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

OrderStatusComboBox::OrderStatusComboBox(QWidget* parent)
    : QWidget(parent) {
  auto menu = new StaticDropDownMenu(make_order_status_list(), this);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->addWidget(menu);
  m_value_connection = menu->connect_value_selected_signal(
    [=] (const auto& value) {
      m_selected_signal(value.value<OrderStatus>());
    });
}

connection OrderStatusComboBox::connect_selected_signal(
    const SelectedSignal::slot_type& slot) const {
  return m_selected_signal.connect(slot);
}
