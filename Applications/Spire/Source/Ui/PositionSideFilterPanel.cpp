#include "Spire/Ui/PositionSideFilterPanel.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

using namespace Nexus;
using namespace Spire;

PositionSideFilterPanel* Spire::make_position_side_filter_panel(
    QWidget* parent) {
  return make_position_side_filter_panel(
    std::make_shared<ArrayListModel<Side>>(), parent);
}

PositionSideFilterPanel* Spire::make_position_side_filter_panel(
    std::shared_ptr<SideListModel> selection, QWidget* parent) {
  auto tokens = std::make_shared<ArrayListModel<PositionSideToken>>();
  for(auto i = 0; i < selection->get_size(); ++i) {
    tokens->push(PositionSideToken(selection->get(i)));
  }
  return make_closed_filter_panel<PositionSideToken>(
    {PositionSideToken(Side::BID), PositionSideToken(Side::ASK),
      PositionSideToken(Side::NONE)}, std::move(tokens), parent);
}
