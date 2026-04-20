#include "Spire/Ui/DestinationListBox.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/LocalQueryModel.hpp"
#include "Spire/Ui/DestinationListItem.hpp"

using namespace Nexus;
using namespace Spire;

DestinationListBox* Spire::make_destination_list_box(QWidget* parent) {
  return make_destination_list_box(
    std::make_shared<ArrayListModel<Destination>>(), parent);
}

DestinationListBox* Spire::make_destination_list_box(
    std::shared_ptr<DestinationListModel> current, QWidget* parent) {
  auto query_model = std::make_shared<LocalQueryModel<Destination>>();
  for(auto& entry : DEFAULT_DESTINATIONS.get_entries()) {
    query_model->add(entry.m_id);
  }
  return new TagComboBox<Destination>(std::move(query_model),
    std::move(current),
    [] (const std::shared_ptr<DestinationListModel>& list, int index) {
      return new DestinationListItem(
        DEFAULT_DESTINATIONS.from(list->get(index)));
    }, parent);
}
