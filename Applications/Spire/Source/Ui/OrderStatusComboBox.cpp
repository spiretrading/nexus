#include "Spire/Ui/OrderStatusComboBox.hpp"
#include <Beam/Collections/EnumIterator.hpp>
#include <QHBoxLayout>
#include "Spire/Ui/StaticDropDownMenu.hpp"

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
  auto menu = new StaticDropDownMenu(items, this);
  menu->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
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
