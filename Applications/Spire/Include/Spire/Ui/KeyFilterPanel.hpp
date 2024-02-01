#ifndef SPIRE_KEY_FILTER_PANEL_HPP
#define SPIRE_KEY_FILTER_PANEL_HPP
#include "Spire/Ui/KeyInputBox.hpp"
#include "Spire/Ui/OpenFilterPanelTemplate.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /**
   * An OpenFilterPanelTemplate specialized for a QKeySequence.
   */
  using KeyFilterPanel = OpenFilterPanelTemplate<QKeySequence>;

  /**
   * Returns a new KeyFilterPanel using default models and an empty list of matches.
   * @param parent The parent widget.
   */
  KeyFilterPanel* make_key_filter_panel(QWidget& parent);

  /**
   * Returns a new KeyFilterPanel using an empty list of matches.
   * @param current The current value model.
   * @param parent The parent widget.
   */
  KeyFilterPanel* make_key_filter_panel(
    std::shared_ptr<KeySequenceValueModel> current, QWidget& parent);

  /**
   * Returns a new KeyFilterPanel.
   * @param current The current value model.
   * @param matches The list of QKeySequence to match against.
   * @param mode The filter mode.
   * @param parent The parent widget.
   */
  KeyFilterPanel* make_key_filter_panel(
    std::shared_ptr<KeySequenceValueModel> current,
    std::shared_ptr<ListModel<QKeySequence>> matches,
    std::shared_ptr<ValueModel<KeyFilterPanel::Mode>> mode,
    QWidget& parent);
}

#endif
