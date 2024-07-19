#include "Spire/Ui/MarketBox.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Ui/DestinationListItem.hpp"

using namespace Nexus;
using namespace Spire;

MarketBox* Spire::make_market_box(std::shared_ptr<MarketModel> current,
    const MarketDatabase& markets, QWidget* parent) {
  auto settings = MarketBox::Settings([=] (const auto& market) {
    return QString::fromStdString(markets.FromCode(market).m_displayName);
  },
  [=] (const auto& market) {
    auto& entry = markets.FromCode(market);
    auto destination_entry = DestinationDatabase::Entry();
    destination_entry.m_id = entry.m_displayName;
    destination_entry.m_description = entry.m_description;
    return new DestinationListItem(std::move(destination_entry));
  });
  auto market_list = std::make_shared<ArrayListModel<MarketCode>>();
  for(auto& market : markets.GetEntries()) {
    market_list->push(market.m_code);
  }
  settings.m_cases = std::move(market_list);
  settings.m_current = std::move(current);
  return new MarketBox(std::move(settings), parent);
}
