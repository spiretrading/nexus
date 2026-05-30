#ifndef SPIRE_TRADING_GROUP_LIST_BOX_HPP
#define SPIRE_TRADING_GROUP_LIST_BOX_HPP
#include "Spire/Ui/TagComboBox.hpp"
#include "Spire/Ui/TradingGroupBox.hpp"

namespace Spire {

  /** Represents a ListModel for a list of trading groups. */
  using TradingGroupListModel = ListModel<Beam::DirectoryEntry>;

  /** A TagComboBox specialized for trading groups. */
  using TradingGroupListBox = TagComboBox<Beam::DirectoryEntry>;

  /**
   * Returns a new TradingGroupListBox using a default current model.
   * @param groups The set of groups that can be queried.
   * @param parent The parent widget.
   */
  TradingGroupListBox* make_trading_group_list_box(
    std::shared_ptr<TradingGroupQueryModel> groups, QWidget* parent = nullptr);

  /**
   * Returns a new TradingGroupListBox.
   * @param groups The set of groups that can be queried.
   * @param current The current list of selected groups.
   * @param parent The parent widget.
   */
  TradingGroupListBox* make_trading_group_list_box(
    std::shared_ptr<TradingGroupQueryModel> groups,
    std::shared_ptr<TradingGroupListModel> current, QWidget* parent = nullptr);
}

#endif
