#ifndef SPIRE_LINK_MENU_HPP
#define SPIRE_LINK_MENU_HPP
#include <vector>
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

  /** Describes a window that a "Link To" sub-menu can target. */
  struct LinkableWindowInfo {

    /** The unique identifier of the window. */
    QString m_id;

    /** The type of linkable window. */
    LinkableWindowType m_type;

    /** The ticker that the window represents. */
    Nexus::Ticker m_ticker;

    auto operator <=>(const LinkableWindowInfo&) const = default;
  };

  /** A ValueModel over an optional LinkableWindowInfo. */
  using OptionalLinkableWindowInfoModel =
    ValueModel<boost::optional<LinkableWindowInfo>>;

  /** A LocalValueModel over an optional LinkableWindowInfo. */
  using LocalOptionalLinkableWindowInfoModel =
    LocalValueModel<boost::optional<LinkableWindowInfo>>;

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

  /**
   * Adds a "Link To" sub-menu.
   * @param parent The ContextMenu to add the sub-menu to.
   * @param windows The candidate linkable windows.
   * @return The linked window model.
   */
  std::shared_ptr<OptionalLinkableWindowInfoModel> add_link_sub_menu_item(
    ContextMenu& parent, std::vector<LinkableWindowInfo> windows);

  /**
   * Adds a "Link To" sub-menu using a caller-provided current model.
   * @param parent The ContextMenu to add the sub-menu to.
   * @param windows The candidate linkable windows.
   * @param current The linked window model.
   */
  void add_link_sub_menu_item(ContextMenu& parent,
    std::vector<LinkableWindowInfo> windows,
    std::shared_ptr<OptionalLinkableWindowInfoModel> current);
}

#endif
