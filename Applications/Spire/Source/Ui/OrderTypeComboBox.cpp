#include "Spire/Ui/OrderTypeComboBox.hpp"
#include <Beam/Collections/EnumIterator.hpp>
#include <QHBoxLayout>

using namespace Beam;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

OrderTypeComboBox::OrderTypeComboBox(QWidget* parent)
    : QLineEdit(parent) {
  setReadOnly(true);
  auto items = [] {
    auto types = std::vector<QVariant>();
    types.reserve(OrderType::COUNT);
    for(auto type : Beam::MakeRange<OrderType>()) {
      types.push_back(QVariant::fromValue(type));
    }
    return types;
  }();
  m_menu = new StaticDropDownMenu(items, this);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->addWidget(m_menu);
  m_value_connection = m_menu->connect_value_selected_signal(
    [=] (const auto& value) {
      m_selected_signal(value.value<OrderType>());
      Q_EMIT editingFinished();
    });
}

OrderType OrderTypeComboBox::get_current_order_type() const {
  return m_menu->get_current_item().value<OrderType>();
}

connection OrderTypeComboBox::connect_selected_signal(
    const SelectedSignal::slot_type& slot) const {
  return m_selected_signal.connect(slot);
}
