#ifndef SPIRE_SIDE_BOX_HPP
#define SPIRE_SIDE_BOX_HPP
#include "Nexus/Definitions/Side.hpp"
#include "Spire/Ui/EnumBox.hpp"

namespace Spire {

  /** A ValueModel over a Nexus::Side. */
  using SideModel = ValueModel<Nexus::Side>;

  /** A LocalValueModel over a Nexus::Side. */
  using LocalSideModel = LocalValueModel<Nexus::Side>;

  /** An EnumBox specialized for a Nexus::Side. */
  using SideBox = EnumBox<Nexus::Side>;

  /**
   * Returns a new SideBox using a LocalSideModel.
   * @param parent The parent widget.
   */
  SideBox* make_side_box(QWidget* parent = nullptr);

  /**
   * Returns a new SideBox using a LocalSideModel and initial current value.
   * @param current The initial current value.
   * @param parent The parent widget.
   */
  SideBox* make_side_box(Nexus::Side current, QWidget* parent = nullptr);

  /**
   * Returns a SideBox.
   * @param model The current value model.
   * @param parent The parent widget.
   */
   SideBox* make_side_box(
    std::shared_ptr<SideModel> current, QWidget* parent = nullptr);
}

#endif
