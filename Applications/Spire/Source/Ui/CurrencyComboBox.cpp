#include "Spire/Ui/CurrencyComboBox.hpp"
#include <QHBoxLayout>
#include "Spire/Ui/StaticDropDownMenu.hpp"

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

CurrencyComboBox::CurrencyComboBox(const CurrencyDatabase& database,
    QWidget* parent)
    : QWidget(parent) {
  auto menu = new StaticDropDownMenu(make_currency_list(database), this);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->addWidget(menu);
  m_value_connection = menu->connect_value_selected_signal(
    [=] (const auto& value) { m_selected_signal(value.value<CurrencyId>()); });
}

connection CurrencyComboBox::connect_selected_signal(
    const SelectedSignal::slot_type& slot) const {
  return m_selected_signal.connect(slot);
}
