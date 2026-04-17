#include "Spire/Ui/DestinationFilterPanel.hpp"
#include "Spire/Spire/ArrayListModel.hpp"

using namespace Nexus;
using namespace Spire;

DestinationFilterPanel* Spire::make_destination_filter_panel(QWidget* parent) {
  return make_destination_filter_panel(
    std::make_shared<ArrayListModel<Destination>>(), parent);
}

DestinationFilterPanel* Spire::make_destination_filter_panel(
    std::shared_ptr<DestinationListModel> current, QWidget* parent) {
  auto box = make_destination_list_box(std::move(current));
  box->set_placeholder(QObject::tr("Enter securities, markets, or countries"));
  return new OpenFilterPanel(*box, parent);
}
