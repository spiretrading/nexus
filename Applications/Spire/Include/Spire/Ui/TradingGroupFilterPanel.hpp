#ifndef SPIRE_TRADING_GROUP_FILTER_PANEL_HPP
#define SPIRE_TRADING_GROUP_FILTER_PANEL_HPP
#include "Spire/Ui/OpenFilterPanel.hpp"
#include "Spire/Ui/TradingGroupListBox.hpp"

namespace Spire {

  /** An OpenFilterPanel specialized for a TradingGroupListBox. */
  using TradingGroupFilterPanel = OpenFilterPanel<TradingGroupListBox>;

  /**
   * Returns a new TradingGroupFilterPanel with a default current model.
   * @param groups The set of groups that can be queried.
   * @param parent The parent widget.
   */
  TradingGroupFilterPanel* make_trading_group_filter_panel(
    std::shared_ptr<TradingGroupQueryModel> groups, QWidget* parent = nullptr);

  /**
   * Returns a new TradingGroupFilterPanel.
   * @param groups The set of groups that can be queried.
   * @param current The current list of selected groups.
   * @param parent The parent widget.
   */
  TradingGroupFilterPanel* make_trading_group_filter_panel(
    std::shared_ptr<TradingGroupQueryModel> groups,
    std::shared_ptr<TradingGroupListModel> current, QWidget* parent = nullptr);
}

#endif
