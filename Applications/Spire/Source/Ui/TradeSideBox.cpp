#include "Spire/Ui/TradeSideBox.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

using namespace Nexus;
using namespace Spire;

TradeSideBox* Spire::make_trade_side_box(QWidget* parent) {
  return make_trade_side_box(Side::BID, parent);
}

TradeSideBox* Spire::make_trade_side_box(Side current, QWidget* parent) {
  return make_trade_side_box(std::make_shared<LocalSideModel>(current), parent);
}

TradeSideBox* Spire::make_trade_side_box(
    std::shared_ptr<SideModel> current, QWidget* parent) {
  auto to_text = [] (const Side& side) {
    return Spire::to_text(TradeSide(side));
  };
  auto view_builder = [=] (const Side& side) {
    return make_label(to_text(side));
  };
  auto settings = TradeSideBox::Settings(to_text, view_builder);
  auto cases = std::make_shared<ArrayListModel<Side>>();
  cases->push(Side::BID);
  cases->push(Side::ASK);
  settings.m_cases = std::move(cases);
  settings.m_current = std::move(current);
  return new TradeSideBox(std::move(settings), parent);
}
