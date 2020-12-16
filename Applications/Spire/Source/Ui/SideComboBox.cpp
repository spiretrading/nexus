#include "Spire/Ui/SideComboBox.hpp"
#include <QHBoxLayout>

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

SideComboBox::SideComboBox(QWidget* parent)
    : QWidget(parent) {
  m_menu = new StaticDropDownMenu({QVariant::fromValue<Side>(Side::ASK),
    QVariant::fromValue<Side>(Side::BID)}, this);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->addWidget(m_menu);
  m_value_connection = m_menu->connect_value_selected_signal(
    [=] (const auto& value) { m_selected_signal(value.value<Side>()); });
}

Side SideComboBox::get_side() const {
  return m_menu->get_current_item().value<Side>();
}

void SideComboBox::set_side(Side side) {
  m_menu->set_current_item(QVariant::fromValue(side));
}

connection SideComboBox::connect_selected_signal(
    const SelectedSignal::slot_type& slot) const {
  return m_selected_signal.connect(slot);
}
