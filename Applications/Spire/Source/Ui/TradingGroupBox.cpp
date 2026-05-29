#include "Spire/Ui/TradingGroupBox.hpp"
#include "Spire/Ui/ListView.hpp"

using namespace Spire;

TradingGroupBox* Spire::make_trading_group_box(
    std::shared_ptr<TradingGroupQueryModel> groups, QWidget* parent) {
  return make_trading_group_box(
    std::move(groups), std::make_shared<LocalTradingGroupModel>(), parent);
}

TradingGroupBox* Spire::make_trading_group_box(
    std::shared_ptr<TradingGroupQueryModel> groups,
    std::shared_ptr<TradingGroupModel> current, QWidget* parent) {
  return new TradingGroupBox(std::move(groups), std::move(current),
    &ListView::default_item_builder, parent);
}
