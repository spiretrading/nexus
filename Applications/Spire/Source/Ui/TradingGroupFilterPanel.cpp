#include "Spire/Ui/TradingGroupFilterPanel.hpp"
#include "Spire/Spire/ArrayListModel.hpp"

using namespace Beam;
using namespace Spire;

TradingGroupFilterPanel* Spire::make_trading_group_filter_panel(
    std::shared_ptr<TradingGroupQueryModel> groups, QWidget* parent) {
  return make_trading_group_filter_panel(std::move(groups),
    std::make_shared<ArrayListModel<DirectoryEntry>>(), parent);
}

TradingGroupFilterPanel* Spire::make_trading_group_filter_panel(
    std::shared_ptr<TradingGroupQueryModel> groups,
    std::shared_ptr<TradingGroupListModel> current, QWidget* parent) {
  auto box = make_trading_group_list_box(std::move(groups), std::move(current));
  box->set_placeholder(QObject::tr("Enter groups"));
  return new OpenFilterPanel(*box, parent);
}
