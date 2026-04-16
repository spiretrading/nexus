#include "Spire/Ui/VenueFilterPanel.hpp"
#include "Spire/Spire/ArrayListModel.hpp"

using namespace Nexus;
using namespace Spire;

VenueFilterPanel* Spire::make_venue_filter_panel(QWidget* parent) {
  return make_venue_filter_panel(
    std::make_shared<ArrayListModel<Venue>>(), parent);
}

VenueFilterPanel* Spire::make_venue_filter_panel(
    std::shared_ptr<VenueListModel> current, QWidget* parent) {
  auto box = make_venue_list_box(std::move(current));
  box->set_placeholder(QObject::tr("Enter venues"));
  return new OpenFilterPanel(*box, parent);
}
