#ifndef SPIRE_TRADE_SIDE_BOX_HPP
#define SPIRE_TRADE_SIDE_BOX_HPP
#include "Spire/Ui/SideBox.hpp"

namespace Spire {

  /** A SideBox for selecting the side of a trade. */
  using TradeSideBox = SideBox;

  /**
   * Returns a TradeSideBox initially set to Side::BID.
   * @param parent The parent widget.
   */
  TradeSideBox* make_trade_side_box(QWidget* parent = nullptr);

  /**
   * Returns a TradeSideBox with an initial side.
   * @param current The initially selected side.
   * @param parent The parent widget.
   */
  TradeSideBox* make_trade_side_box(
    Nexus::Side current, QWidget* parent = nullptr);

  /**
   * Returns a TradeSideBox.
   * @param current The model holding the selected side.
   * @param parent The parent widget.
   */
  TradeSideBox* make_trade_side_box(
    std::shared_ptr<SideModel> current, QWidget* parent = nullptr);
}

#endif
