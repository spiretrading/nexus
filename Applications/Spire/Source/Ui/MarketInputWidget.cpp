#include "Spire/Ui/MarketInputWidget.hpp"
#include "Nexus/Definitions/Market.hpp"

using namespace Nexus;
using namespace Spire;

MarketInputWidget::MarketInputWidget(QWidget* parent)
  : FilteredDropDownMenu([] {
      auto entries = GetDefaultMarketDatabase().GetEntries();
      auto items = [&] {
        auto markets = std::vector<QVariant>();
        markets.reserve(entries.size());
        std::transform(entries.begin(), entries.end(),
          std::back_inserter(markets), [] (const auto& entry) {
            return QVariant::fromValue<MarketToken>(entry.m_code);
          });
        return markets;
      }();
      return items;
    }(), parent) {}

MarketToken MarketInputWidget::get_market() const {
  return get_item().value<MarketToken>();
}
