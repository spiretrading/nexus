#ifndef SPIRE_QUANTITY_SETTING_BOX_HPP
#define SPIRE_QUANTITY_SETTING_BOX_HPP
#include "Spire/KeyBindings/OrderTaskArguments.hpp"
#include "Spire/Ui/EnumBox.hpp"

namespace Spire {  

  /** A ValueModel over a QuantitySetting. */
  using QuantitySettingModel = ValueModel<QuantitySetting>;

  /** A LocalQuantitySettingModel over a QuantitySetting. */
  using LocalQuantitySettingModel = LocalValueModel<QuantitySetting>;

  /** An EnumBox specialized for a QuantitySetting. */
  using QuantitySettingBox = EnumBox<QuantitySetting>;

  /**
   * Returns a new QuantitySettingBox using a LocalQuantitySettingModel.
   * @param parent The parent widget.
   */
  QuantitySettingBox* make_quantity_setting_box(QWidget* parent = nullptr);

  /**
   * Returns a new QuantitySettingBox using a LocalQuantitySettingModel and
   * initial current value.
   * @param current The initial current value.
   * @param parent The parent widget.
   */
  QuantitySettingBox* make_quantity_setting_box(
    QuantitySetting current, QWidget* parent = nullptr);

  /**
   * Returns a QuantitySettingBox.
   * @param current The current value.
   * @param parent The parent widget.
   */
   QuantitySettingBox* make_quantity_setting_box(
    std::shared_ptr<QuantitySettingModel> current, QWidget* parent = nullptr);
}

#endif
