#ifndef SPIRE_LINK_MENU_HPP
#define SPIRE_LINK_MENU_HPP
#include "Spire/LegacyUI/SecurityContext.hpp"
#include "Spire/Ui/ContextMenu.hpp"
#include "Spire/Utilities/Utilities.hpp"

namespace Spire {

  /**
   * Adds a ContextMenu sub-menu with actions to link a window to another.
   * @param parent The ContextMenu to add the sub-menu to.
   * @param window The window representing the SecurityContext to link.
   * @param market_database The database of markets used to sort ticker symbols.
   */
  void add_link_menu(ContextMenu& parent, LegacyUI::SecurityContext& window,
    const Nexus::MarketDatabase& market_database);
}

#endif
