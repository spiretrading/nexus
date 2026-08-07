#include "Spire/Ui/TradingGroupListBox.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Ui/ListView.hpp"

using namespace Beam;
using namespace Spire;

TradingGroupListBox* Spire::make_trading_group_list_box(
    std::shared_ptr<TradingGroupQueryModel> groups, QWidget* parent) {
  return make_trading_group_list_box(std::move(groups),
    std::make_shared<ArrayListModel<DirectoryEntry>>(), parent);
}

TradingGroupListBox* Spire::make_trading_group_list_box(
    std::shared_ptr<TradingGroupQueryModel> groups,
    std::shared_ptr<TradingGroupListModel> current, QWidget* parent) {
  return new TradingGroupListBox(std::move(groups), std::move(current),
    &ListView::default_item_builder, parent);
}
