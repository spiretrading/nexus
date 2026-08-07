#ifndef SPIRE_TRADING_GROUP_BOX_HPP
#define SPIRE_TRADING_GROUP_BOX_HPP
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/ComboBox.hpp"

namespace Spire {

  /** Represents a QueryModel for trading groups. */
  using TradingGroupQueryModel = QueryModel<Beam::DirectoryEntry>;

  /** Represents a ValueModel over a trading group. */
  using TradingGroupModel = ValueModel<Beam::DirectoryEntry>;

  /** Represents a LocalValueModel over a trading group. */
  using LocalTradingGroupModel = LocalValueModel<Beam::DirectoryEntry>;

  /** A ComboBox specialized for trading groups. */
  using TradingGroupBox = ComboBox<Beam::DirectoryEntry>;

  /**
   * Returns a new TradingGroupBox using a default current model.
   * @param groups The set of groups that can be queried.
   * @param parent The parent widget.
   */
  TradingGroupBox* make_trading_group_box(
    std::shared_ptr<TradingGroupQueryModel> groups, QWidget* parent = nullptr);

  /**
   * Returns a new TradingGroupBox.
   * @param groups The set of groups that can be queried.
   * @param current The current group displayed.
   * @param parent The parent widget.
   */
  TradingGroupBox* make_trading_group_box(
    std::shared_ptr<TradingGroupQueryModel> groups,
    std::shared_ptr<TradingGroupModel> current, QWidget* parent = nullptr);
}

#endif
