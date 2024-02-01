#ifndef SPIRE_FONT_STYLE_BOX_HPP
#define SPIRE_FONT_STYLE_BOX_HPP
#include "Spire/Ui/EnumBox.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** An EnumBox specialized for the font style. */
  using FontStyleBox = EnumBox<QString>;

  /**
   * Returns a new FontStyleBox using the initial style which is "Regular"
   * or the equivalent in the font family.
   * @param font_family The font family that the font style box represents.
   * @param parent The parent widget.
   */
  FontStyleBox* make_font_style_box(
    std::shared_ptr<ValueModel<QString>> font_family,
    QWidget* parent = nullptr);

  /**
   * Returns a FontStyleBox.
   * @param font_family The font family that the font style box represents.
   * @param current The current font style.
   * @param parent The parent widget.
   */
   FontStyleBox* make_font_style_box(
    std::shared_ptr<ValueModel<QString>> font_family,
    std::shared_ptr<ValueModel<QString>> current, QWidget* parent = nullptr);
}

#endif
