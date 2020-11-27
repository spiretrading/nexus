#include "Spire/Ui/MarketComboBox.hpp"
#include <algorithm>
#include <QHBoxLayout>

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
  m_menu = new StaticDropDownMenu(items, this);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->addWidget(m_menu);
  m_value_connection = m_menu->connect_value_selected_signal(
    [=] (const auto& value) {
      m_selected_signal(value.value<MarketToken>().m_code);
    });
}

MarketCode MarketComboBox::get_market() const {
  return m_menu->get_current_item().value<MarketToken>().m_code;
}

void MarketComboBox::set_market(MarketCode market) {
  m_menu->set_current_item(QVariant::fromValue<MarketToken>(market));
}

connection MarketComboBox::connect_selected_signal(
    const SelectedSignal::slot_type& slot) const {
  return m_selected_signal.connect(slot);
}
