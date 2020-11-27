#include "Spire/Ui/CurrencyComboBox.hpp"
#include <algorithm>
#include <QHBoxLayout>

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

CurrencyComboBox::CurrencyComboBox(const CurrencyDatabase& database,
    QWidget* parent)
    : QLineEdit(parent) {
  setReadOnly(true);
  auto entries = database.GetEntries();
  auto items = [&] {
    auto currencies = std::vector<QVariant>();
    currencies.reserve(entries.size());
    std::transform(entries.begin(), entries.end(),
      std::back_inserter(currencies), [] (const auto& entry) {
        return QVariant::fromValue(entry.m_id);
      });
    return currencies;
  }();
  m_menu = new StaticDropDownMenu(items, this);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->addWidget(m_menu);
  m_value_connection = m_menu->connect_value_selected_signal(
    [=] (const auto& value) { Q_EMIT editingFinished(); });
}

CurrencyId CurrencyComboBox::get_currency() const {
  return m_menu->get_current_item().value<CurrencyId>();
}

void CurrencyComboBox::set_currency(CurrencyId currency) {
  m_menu->set_current_item(QVariant::fromValue(currency));
}
