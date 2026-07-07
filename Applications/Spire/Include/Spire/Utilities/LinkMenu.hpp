#ifndef SPIRE_LINK_MENU_HPP
#define SPIRE_LINK_MENU_HPP
#include "Spire/LegacyUI/TickerContext.hpp"
#include "Spire/Ui/ContextMenu.hpp"

namespace Spire {
  class CheckButtonMenuItem;

  /** Kinds of window that a "Link To" menu can target. */
  enum class LinkableWindowType {

    /** A book view window. */
    BOOK_VIEW,

    /** A chart window. */
    CHART,

    /** A time and sales window. */
    TIME_AND_SALES
  };

  /**
   * Adds a ContextMenu sub-menu with actions to link a window to another.
   * @param parent The ContextMenu to add the sub-menu to.
   * @param window The window representing the TickerContext to link.
   */
  void add_link_menu(ContextMenu& parent, LegacyUI::TickerContext& window);

  /**
   * Returns a CheckButtonMenuItem representing a linkable window with
   * a default current.
   * @param type The type of linkable window.
   * @param ticker The ticker displayed by that window.
   * @param parent The parent widget.
   */
  CheckButtonMenuItem* make_link_menu_item(LinkableWindowType type,
    const Nexus::Ticker& ticker, QWidget* parent = nullptr);

  /**
   * Returns a CheckButtonMenuItem representing a linkable window.
   * @param type The type of linkable window.
   * @param ticker The ticker displayed by that window.
   * @param current Whether the item is checked.
   * @param parent The parent widget.
   */
  CheckButtonMenuItem* make_link_menu_item(LinkableWindowType type,
    const Nexus::Ticker& ticker, std::shared_ptr<BooleanModel> current,
    QWidget* parent = nullptr);
}

#endif
