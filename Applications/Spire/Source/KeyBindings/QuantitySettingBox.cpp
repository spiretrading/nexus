#include "Spire/KeyBindings/QuantitySettingBox.hpp"

using namespace Spire;

namespace {
  QuantitySettingBox::Settings setup() {
    static auto settings = [] {
      auto settings = QuantitySettingBox::Settings();
      auto cases = std::make_shared<ArrayListModel<QuantitySetting>>();
      cases->push(QuantitySetting::DEFAULT);
      cases->push(QuantitySetting::ADJUSTABLE);
      settings.m_cases = std::move(cases);
      return settings;
    }();
    return settings;
  }
}

QuantitySettingBox* Spire::make_quantity_setting_box(QWidget* parent) {
  return make_quantity_setting_box(
    std::make_shared<LocalQuantitySettingModel>(), parent);
}

QuantitySettingBox* Spire::make_quantity_setting_box(
    QuantitySetting current, QWidget* parent) {
  return make_quantity_setting_box(
    std::make_shared<LocalQuantitySettingModel>(current), parent);
}

QuantitySettingBox* Spire::make_quantity_setting_box(
    std::shared_ptr<QuantitySettingModel> current, QWidget* parent) {
  auto settings = setup();
  settings.m_current = std::move(current);
  return new QuantitySettingBox(std::move(settings), parent);
}
