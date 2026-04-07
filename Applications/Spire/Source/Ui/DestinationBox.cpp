#include "Spire/Ui/DestinationBox.hpp"
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/DefaultVenueDatabase.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/FilteredListModel.hpp"
#include "Spire/Spire/TransformListModel.hpp"
#include "Spire/Ui/DestinationListItem.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

namespace {
  auto make_filter(const Region& region) {
    return [=] (const auto& list, auto index) {
      if(region.is_global()) {
        return false;
      }
      auto& entry = list.get(index);
      for(auto& venue : region.get_venues()) {
        auto i = std::find(entry.m_venues.begin(), entry.m_venues.end(), venue);
        if(i == entry.m_venues.end()) {
          return true;
        }
      }
      for(auto& country : region.get_countries()) {
        auto i = std::find_if(entry.m_venues.begin(), entry.m_venues.end(),
          [&] (const auto& venue) {
            return DEFAULT_VENUES.from(venue).m_country_code == country;
          });
        if(i == entry.m_venues.end()) {
          return true;
        }
      }
      return false;
    };
  }

  struct RegionFilteredList : FilteredListModel<DestinationDatabase::Entry> {
    std::shared_ptr<RegionModel> m_region;
    scoped_connection m_connection;

    RegionFilteredList(
        std::shared_ptr<ListModel<DestinationDatabase::Entry>> destinations,
        std::shared_ptr<RegionModel> region)
        : FilteredListModel(
            std::move(destinations), make_filter(region->get())),
          m_region(std::move(region)) {
      m_connection = m_region->connect_update_signal(
        std::bind_front(&RegionFilteredList::on_region, this));
    }

    void on_region(const Region& region) {
      set_filter(make_filter(region));
    }
  };
}

std::shared_ptr<ListModel<DestinationDatabase::Entry>>
    Spire::make_region_filtered_destination_list(
      std::shared_ptr<RegionModel> region) {
  auto selection =
    DEFAULT_DESTINATIONS.select_all([] (const auto&) { return true; });
  auto full_list =
    std::make_shared<ArrayListModel<DestinationDatabase::Entry>>(selection);
  return std::make_shared<RegionFilteredList>(
    std::move(full_list), std::move(region));
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
