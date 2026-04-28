#include "Spire/Ui/PositionSideBox.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

using namespace Nexus;
using namespace Spire;

namespace {
  PositionSideBox::Settings setup() {
    static auto settings = [] {
      auto to_text = [] (const Side& side) {
        return Spire::to_text(PositionSideToken(side));
      };
      auto view_builder = [=] (const Side& side) {
        return make_label(to_text(side));
      };
      auto settings = PositionSideBox::Settings(to_text, view_builder);
      auto cases = std::make_shared<ArrayListModel<Side>>();
      cases->push(Side::BID);
      cases->push(Side::ASK);
      cases->push(Side::NONE);
      settings.m_cases = std::move(cases);
      return settings;
    }();
    return settings;
  }
}

PositionSideBox* Spire::make_position_side_box(QWidget* parent) {
  return make_position_side_box(Side::BID, parent);
}

PositionSideBox* Spire::make_position_side_box(Side current, QWidget* parent) {
  return make_position_side_box(
    std::make_shared<LocalSideModel>(current), parent);
}

PositionSideBox* Spire::make_position_side_box(
    std::shared_ptr<SideModel> current, QWidget* parent) {
  auto settings = setup();
  settings.m_current = std::move(current);
  return new SideBox(std::move(settings), parent);
}
