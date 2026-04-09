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
  auto make_filter(const Scope& scope) {
    return [=] (const auto& list, auto index) {
      if(scope.is_global()) {
        return false;
      }
      auto& entry = list.get(index);
      for(auto& venue : scope.get_venues()) {
        auto i = std::find(entry.m_venues.begin(), entry.m_venues.end(), venue);
        if(i == entry.m_venues.end()) {
          return true;
        }
      }
      for(auto& country : scope.get_countries()) {
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

  struct ScopeFilteredList : FilteredListModel<DestinationDatabase::Entry> {
    std::shared_ptr<ScopeModel> m_scope;
    scoped_connection m_connection;

    ScopeFilteredList(
        std::shared_ptr<ListModel<DestinationDatabase::Entry>> destinations,
        std::shared_ptr<ScopeModel> scope)
        : FilteredListModel(
            std::move(destinations), make_filter(scope->get())),
          m_scope(std::move(scope)) {
      m_connection = m_scope->connect_update_signal(
        std::bind_front(&ScopeFilteredList::on_scope, this));
    }

    void on_scope(const Scope& scope) {
      set_filter(make_filter(scope));
    }
  };
}

std::shared_ptr<ListModel<DestinationDatabase::Entry>>
    Spire::make_scope_filtered_destination_list(
      std::shared_ptr<ScopeModel> scope) {
  auto selection =
    DEFAULT_DESTINATIONS.select_all([] (const auto&) { return true; });
  auto full_list =
    std::make_shared<ArrayListModel<DestinationDatabase::Entry>>(selection);
  return std::make_shared<ScopeFilteredList>(
    std::move(full_list), std::move(scope));
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
