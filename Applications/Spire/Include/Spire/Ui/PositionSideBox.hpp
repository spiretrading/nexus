#ifndef SPIRE_POSITION_SIDE_BOX_HPP
#define SPIRE_POSITION_SIDE_BOX_HPP
#include "Spire/Ui/SideBox.hpp"

namespace Spire {

  /** Type alias for a SideBox displaying position sides. */
  using PositionSideBox = SideBox;

  /**
   * Returns a PositionSideBox using a LocalSideModel.
   * @param parent The parent widget.
   */
  PositionSideBox* make_position_side_box(QWidget* parent = nullptr);

  /**
   * Returns a PositionSideBox using a LocalSideModel and initial current value.
   * @param current The initial current value.
   * @param parent The parent widget.
   */
  PositionSideBox* make_position_side_box(
    Nexus::Side current, QWidget* parent = nullptr);

  /**
   * Returns a PositionSideBox.
   * @param current The current value model.
   * @param parent The parent widget.
   */
  PositionSideBox* make_position_side_box(
    std::shared_ptr<SideModel> current, QWidget* parent = nullptr);
}

#endif
