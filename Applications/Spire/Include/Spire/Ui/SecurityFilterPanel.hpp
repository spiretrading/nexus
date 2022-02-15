#ifndef SPIRE_SECURITY_FILTER_PANEL_HPP
#define SPIRE_SECURITY_FILTER_PANEL_HPP
#include "Nexus/Definitions/Security.hpp"
#include "Spire/Ui/OpenFilterPanelTemplate.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /**
   * An OpenFilterPanelTemplate specialized for a Nexus::Security.
   */
  using SecurityFilterPanel = OpenFilterPanelTemplate<Nexus::Security>;

  /**
   * Returns a new SecurityFilterPanel using an empty list of matches.
   * @param query_model The model used to query matches.
   * @param parent The parent widget.
   */
  SecurityFilterPanel* make_security_filter_panel(
    std::shared_ptr<ComboBox::QueryModel> query_model, QWidget& parent);

  /**
   * Returns a new SecurityFilterPanel.
   * @param query_model The model used to query matches.
   * @param matches The list of secuirty values to match against.
   * @param mode The filter mode.
   * @param parent The parent widget.
   */
  SecurityFilterPanel* make_security_filter_panel(
    std::shared_ptr<ComboBox::QueryModel> query_model,
    std::shared_ptr<ListModel<Nexus::Security>> matches,
    std::shared_ptr<ValueModel<SecurityFilterPanel::Mode>> mode,
    QWidget& parent);
}

#endif
