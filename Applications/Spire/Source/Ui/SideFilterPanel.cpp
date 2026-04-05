#include "Spire/Ui/SideFilterPanel.hpp"
#include "Spire/Spire/ArrayListModel.hpp"

using namespace Nexus;
using namespace Spire;

SideFilterPanel* Spire::make_side_filter_panel(QWidget* parent) {
  return make_side_filter_panel(
    std::make_shared<ArrayListModel<Side>>(), parent);
}

SideFilterPanel* Spire::make_side_filter_panel(
    std::shared_ptr<SideListModel> selection, QWidget* parent) {
  return make_closed_filter_panel(std::vector<Side>{Side::BID, Side::ASK},
    std::move(selection), parent);
}
