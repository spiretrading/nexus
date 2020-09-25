#include "Spire/Ui/SideInputWidget.hpp"
#include <QHBoxLayout>
#include "Spire/Ui/StaticDropDownMenu.hpp"

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

SideInputWidget::SideInputWidget(QWidget* parent)
    : QWidget(parent) {
  auto menu = new StaticDropDownMenu({QVariant::fromValue<Side>(Side::ASK),
        QVariant::fromValue<Side>(Side::BID)}, this);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->addWidget(menu);
  m_value_connection = menu->connect_value_selected_signal(
    [=] (const auto& value) { m_side_signal(value.value<Side>()); });
}

connection SideInputWidget::connect_side_signal(
    const SideSignal::slot_type& slot) const {
  return m_side_signal.connect(slot);
}
