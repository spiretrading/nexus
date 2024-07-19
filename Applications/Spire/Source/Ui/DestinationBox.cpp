#include "Spire/Ui/DestinationBox.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/FilteredListModel.hpp"
#include "Spire/Spire/TransformListModel.hpp"
#include "Spire/Ui/DestinationListItem.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

namespace {
  auto make_filter(const MarketDatabase& markets, const Region& region) {
    auto filter = [=, &markets] (const auto& list, auto index) {
      if(region.IsGlobal()) {
        return false;
      }
      auto& entry = list.get(index);
      for(auto& market : region.GetMarkets()) {
        auto i =
          std::find(entry.m_markets.begin(), entry.m_markets.end(), market);
        if(i == entry.m_markets.end()) {
          return true;
        }
      }
      for(auto& country : region.GetCountries()) {
        auto i = std::find_if(entry.m_markets.begin(), entry.m_markets.end(),
          [&] (const auto& market) {
            return markets.FromCode(market).m_countryCode == country;
          });
        if(i == entry.m_markets.end()) {
          return true;
        }
      }
      return false;
    };
    return filter;
  }

  struct RegionFilteredList : FilteredListModel<DestinationDatabase::Entry> {
    MarketDatabase m_markets;
    std::shared_ptr<RegionModel> m_region;
    scoped_connection m_connection;

    RegionFilteredList(
        std::shared_ptr<ListModel<DestinationDatabase::Entry>> destinations,
        MarketDatabase markets, std::shared_ptr<RegionModel> region)
        : FilteredListModel(
            std::move(destinations), make_filter(markets, region->get())),
          m_markets(std::move(markets)),
          m_region(std::move(region)) {
      m_connection = m_region->connect_update_signal(
        std::bind_front(&RegionFilteredList::on_region, this));
    }

    void on_region(const Region& region) {
      set_filter(make_filter(m_markets, region));
    }
  };
}

std::shared_ptr<ListModel<DestinationDatabase::Entry>>
    Spire::make_region_filtered_destination_list(
      DestinationDatabase destinations, MarketDatabase markets,
      std::shared_ptr<RegionModel> region) {
  auto selection =
    destinations.SelectEntries([] (const auto&) { return true; });
  auto full_list =
    std::make_shared<ArrayListModel<DestinationDatabase::Entry>>(selection);
  return std::make_shared<RegionFilteredList>(
    std::move(full_list), std::move(markets), std::move(region));
}

DestinationBox* Spire::make_destination_box(
    std::shared_ptr<DestinationModel> current,
    std::shared_ptr<ListModel<DestinationDatabase::Entry>> destinations,
    QWidget* parent) {
  auto settings = DestinationBox::Settings();
  settings.m_cases = make_transform_list_model(destinations,
    [] (const auto& destination) {
      return destination.m_id;
    });
  settings.m_current = std::move(current);
  settings.m_view_builder = [=] (const auto& destination) {
    auto entry = std::find_if(destinations->begin(), destinations->end(),
      [&] (const DestinationDatabase::Entry& entry) {
        return entry.m_id == destination;
      });
    if(entry == destinations->end()) {
      return new DestinationListItem(DestinationDatabase::Entry());
    }
    return new DestinationListItem(*entry);
  };
  return new DestinationBox(std::move(settings), parent);
}
