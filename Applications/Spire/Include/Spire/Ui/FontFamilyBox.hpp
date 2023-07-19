#ifndef SPIRE_FONT_FAMILY_BOX_HPP
#define SPIRE_FONT_FAMILY_BOX_HPP
#include "Spire/Ui/EnumBox.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** An EnumBox specialized for the font family. */
  using FontFamilyBox = EnumBox<QString>;

  /**
   * Returns a new FontFamilyBox using the available font families in the
   * underlying window system and an initial current font family.
   * @param current The initial current font family.
   * @param parent The parent widget.
   */
  FontFamilyBox* make_font_family_box(const QString& current,
    QWidget* parent = nullptr);

  /**
   * Returns a FontFamilyBox.
   * @param families A set of all available font families.
   * @param current The current font family.
   * @param parent The parent widget.
   */
  FontFamilyBox* make_font_family_box(
    std::shared_ptr<ListModel<QString>> families,
    std::shared_ptr<ValueModel<QString>> current, QWidget* parent = nullptr);
}

#endif
