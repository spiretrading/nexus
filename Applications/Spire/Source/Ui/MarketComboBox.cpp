#include "Spire/Ui/MarketComboBox.hpp"
#include <algorithm>
#include <QHBoxLayout>
#include "Spire/Ui/StaticDropDownMenu.hpp"

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

MarketComboBox::MarketComboBox(const MarketDatabase& database,
    QWidget* parent)
    : QWidget(parent) {
  auto entries = database.GetEntries();
  auto items = [&] {
    auto markets = std::vector<QVariant>();
    markets.reserve(entries.size());
    std::transform(entries.begin(), entries.end(),
      std::back_inserter(markets), [] (const auto& entry) {
        return QVariant::fromValue<MarketToken>(entry.m_code);
      });
    return markets;
  }();
  auto menu = new StaticDropDownMenu(items, this);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->addWidget(menu);
  m_value_connection = menu->connect_value_selected_signal(
    [=] (const auto& value) {
      m_selected_signal(value.value<MarketToken>().m_code);
    });
}

connection MarketComboBox::connect_selected_signal(
    const SelectedSignal::slot_type& slot) const {
  return m_selected_signal.connect(slot);
}
