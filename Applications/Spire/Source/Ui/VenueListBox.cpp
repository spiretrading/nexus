#include "Spire/Ui/VenueListBox.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/LocalQueryModel.hpp"
#include "Spire/Ui/DestinationListItem.hpp"

using namespace Nexus;
using namespace Spire;

VenueListBox* Spire::make_venue_list_box(QWidget* parent) {
  return make_venue_list_box(std::make_shared<ArrayListModel<Venue>>(), parent);
}

VenueListBox* Spire::make_venue_list_box(
    std::shared_ptr<VenueListModel> current, QWidget* parent) {
  auto query_model = std::make_shared<LocalQueryModel<Venue>>();
  for(auto& entry : DEFAULT_VENUES.get_entries()) {
    query_model->add(entry.m_venue);
  }
  return new TagComboBox<Venue>(std::move(query_model), std::move(current),
    [] (const std::shared_ptr<VenueListModel>& list, int index) {
      auto& entry = DEFAULT_VENUES.from(list->get(index));
      auto destination_entry = DestinationDatabase::Entry();
      destination_entry.m_id = entry.m_display_name;
      destination_entry.m_description = entry.m_description;
      return new DestinationListItem(std::move(destination_entry));
    }, parent);
}
