#include "Spire/Ui/SideFilterPanel.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/ArrayTableModel.hpp"

using namespace Nexus;
using namespace Spire;

namespace {
  const auto& get_sides() {
    static const auto values = std::vector<Side>{Side::BID, Side::ASK};
    return values;
  }
}

SideFilterPanel* Spire::make_side_filter_panel(QWidget* parent) {
  return make_side_filter_panel(
    std::make_shared<ArrayListModel<Side>>(), parent);
}

SideFilterPanel* Spire::make_side_filter_panel(
    std::shared_ptr<SideListModel> selection, QWidget* parent) {
  return make_closed_filter_panel(get_sides(), std::move(selection), parent);
}
