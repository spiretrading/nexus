#include "Spire/Ui/RegionDropDownBox.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/RegionListItem.hpp"

using namespace Nexus;
using namespace Spire;

RegionDropDownBox* Spire::make_region_drop_down_box(
    std::shared_ptr<RegionListModel> regions, QWidget* parent) {
  return make_region_drop_down_box(std::move(regions),
    std::make_shared<LocalRegionModel>(), parent);
}

RegionDropDownBox* Spire::make_region_drop_down_box(
    std::shared_ptr<RegionListModel> regions,
    std::shared_ptr<RegionModel> current, QWidget* parent) {
  auto settings = RegionDropDownBox::Settings();
  settings.m_cases = std::move(regions);
  settings.m_current = std::move(current);
  settings.m_view_builder = [] (const auto& region) {
    return new RegionListItem(region);
  };
  return new RegionDropDownBox(std::move(settings), parent);
}
