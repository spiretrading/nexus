#ifndef SPIRE_REGION_DROP_DOWN_BOX_HPP
#define SPIRE_REGION_DROP_DOWN_BOX_HPP
#include "Nexus/Definitions/Region.hpp"
#include "Spire/Ui/EnumBox.hpp"

namespace Spire {

  /** A ValueModel over a Nexus::Region. */
  using RegionModel = ValueModel<Nexus::Region>;

  /** A LocalValueModel over a Nexus::Region. */
  using LocalRegionModel = LocalValueModel<Nexus::Region>;

  /** A ListModel over Nexus::Region. */
  using RegionListModel = ListModel<Nexus::Region>;

  /** An EnumBox specialized for a Nexus::Region. */
  using RegionDropDownBox = EnumBox<Nexus::Region>;

  /**
   * Returns a new RegionDropDownBox using a LocalRegionModel.
   * @param regions A closed set of regions.
   * @param parent The parent widget.
   */
  RegionDropDownBox* make_region_drop_down_box(
    std::shared_ptr<RegionListModel> regions, QWidget* parent = nullptr);

  /**
   * Returns a RegionDropDownBox.
   * @param regions A closed set of regions.
   * @param model The current value model.
   * @param parent The parent widget.
   */
   RegionDropDownBox* make_region_drop_down_box(
    std::shared_ptr<RegionListModel> regions,
    std::shared_ptr<RegionModel> current, QWidget* parent = nullptr);
}

#endif
