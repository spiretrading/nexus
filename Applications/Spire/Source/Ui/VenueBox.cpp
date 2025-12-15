#include "Spire/Ui/VenueBox.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/SortedListModel.hpp"
#include "Spire/Ui/DestinationListItem.hpp"

using namespace Nexus;
using namespace Spire;

VenueBox* Spire::make_venue_box(
    std::shared_ptr<VenueModel> current, QWidget* parent) {
  auto settings = VenueBox::Settings(
    [=] (auto venue) {
      return QString::fromStdString(DEFAULT_VENUES.from(venue).m_display_name);
    },
    [=] (auto venue) {
      auto& entry = DEFAULT_VENUES.from(venue);
      auto destination_entry = DestinationDatabase::Entry();
      destination_entry.m_id = entry.m_display_name;
      destination_entry.m_description = entry.m_description;
      return new DestinationListItem(std::move(destination_entry));
    });
  auto venue_list = std::make_shared<SortedListModel<Venue>>(
    std::make_shared<ArrayListModel<Venue>>(),
    [=] (const auto& left, const auto& right) {
      return DEFAULT_VENUES.from(left).m_display_name <
        DEFAULT_VENUES.from(right).m_display_name;
    });
  for(auto& venue : DEFAULT_VENUES.get_entries()) {
    venue_list->push(venue.m_venue);
  }
  settings.m_cases = std::move(venue_list);
  settings.m_current = std::move(current);
  return new VenueBox(std::move(settings), parent);
}
