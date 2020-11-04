#include "Spire/Ui/SideComboBox.hpp"
#include <QHBoxLayout>
#include "Spire/Ui/StaticDropDownMenu.hpp"

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

SideComboBox::SideComboBox(QWidget* parent)
    : QWidget(parent) {
  auto menu = new StaticDropDownMenu({QVariant::fromValue<Side>(Side::ASK),
    QVariant::fromValue<Side>(Side::BID)}, this);
  menu->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->addWidget(menu);
  m_value_connection = menu->connect_value_selected_signal(
    [=] (const auto& value) { m_selected_signal(value.value<Side>()); });
}

connection SideComboBox::connect_selected_signal(
    const SelectedSignal::slot_type& slot) const {
  return m_selected_signal.connect(slot);
}
