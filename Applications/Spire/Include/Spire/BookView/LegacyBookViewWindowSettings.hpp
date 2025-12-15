#ifndef SPIRE_LEGACY_BOOK_VIEW_WINDOW_SETTINGS_HPP
#define SPIRE_LEGACY_BOOK_VIEW_WINDOW_SETTINGS_HPP
#include <unordered_map>
#include <vector>
#include <QByteArray>
#include <QColor>
#include <QFont>
#include "Nexus/Definitions/Security.hpp"
#include "Spire/LegacyUI/SecurityViewStack.hpp"
#include "Spire/LegacyUI/WindowSettings.hpp"
#include "Spire/Spire/ShuttleQtTypes.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /** 
   * Stores the legacy window settings for a BookViewWindow. Provided for
   * backward compatibility with Spire Classic.
   */
  class LegacyBookViewWindowSettings : public LegacyUI::WindowSettings {
    public:

      /** Stores the book view properties used by Spire Classic. */
      struct Properties {

        /** Stores a venue's highlight properties. */
        struct VenueHighlight {

          /** The color to highlight the venue with. */
          QColor m_color;

          /** Whether to highlight all levels. */
          bool m_highlight_all_levels;

          template<Beam::IsShuttle S>
          void shuttle(S& shuttle, unsigned int version);
        };

        /** Stores options available to highlight Orders. */
        enum OrderHighlight {

          /** Do not display Orders. */
          HIDE_ORDERS,

          /** Display Orders using the BookQuote colors. */
          DISPLAY_ORDERS,

          /** Display Orders using a specified color. */
          HIGHLIGHT_ORDERS
        };

        /** The foreground color of a BookQuote. */
        QColor m_book_quote_foreground_color;

        /** The list of background colors used for each level of a BookQuote. */
        std::vector<QColor> m_book_quote_background_colors;

        /** The font used for the BboQuote. */
        QFont m_bbo_quote_font;

        /** The font to used by BookQuotes. */
        QFont m_book_quote_font;

        /** Associates with each venue code a highlight. */
        std::unordered_map<Nexus::Venue, VenueHighlight> m_venue_highlights;

        /** The OrderHighlight option. */
        OrderHighlight m_order_highlight;

        /** The OrderHighlight color. */
        QColor m_order_highlight_color;

        /** <code>true</code> iff grid lines should be displayed. */
        bool m_show_grid;

        /** <code>true</code> iff the BBO should be displayed. */
        bool m_show_bbo;

        template<Beam::IsShuttle S>
        void shuttle(S& shuttle, unsigned int version);
      };

      /** Constructs a LegacyBookViewWindowSettings with default values. */
      LegacyBookViewWindowSettings() = default;

      std::string GetName() const override;

      QWidget* Reopen(Beam::Ref<UserProfile> userProfile) const override;

      void Apply(Beam::Ref<UserProfile> userProfile,
        Beam::Out<QWidget> widget) const override;

    private:
      friend struct Beam::DataShuttle;
      Properties m_properties;
      Nexus::Security m_security;
      std::string m_name;
      LegacyUI::SecurityViewStack m_security_view_stack;
      std::string m_identifier;
      std::string m_link_identifier;
      QByteArray m_geometry;
      QByteArray m_bid_panel_header;
      QByteArray m_ask_panel_header;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void LegacyBookViewWindowSettings::Properties::VenueHighlight::shuttle(
      S& shuttle, unsigned int version) {
    shuttle.shuttle("color", m_color);
    shuttle.shuttle("highlight_all_levels", m_highlight_all_levels);
  }

  template<Beam::IsShuttle S>
  void LegacyBookViewWindowSettings::Properties::shuttle(
      S& shuttle, unsigned int version) {
    shuttle.shuttle(
      "book_quote_foreground_color", m_book_quote_foreground_color);
    shuttle.shuttle(
      "book_quote_background_colors", m_book_quote_background_colors);
    shuttle.shuttle("bbo_quote_font", m_bbo_quote_font);
    shuttle.shuttle("book_quote_font", m_book_quote_font);
    shuttle.shuttle("venue_highlights", m_venue_highlights);
    shuttle.shuttle("order_highlight", m_order_highlight);
    shuttle.shuttle("order_highlight_color", m_order_highlight_color);
    shuttle.shuttle("show_grid", m_show_grid);
    if(version >= 2) {
      shuttle.shuttle("show_bbo", m_show_bbo);
    } else {
      m_show_bbo = false;
    }
  }

  template<Beam::IsShuttle S>
  void LegacyBookViewWindowSettings::shuttle(S& shuttle, unsigned int version) {
    shuttle.shuttle("properties", m_properties);
    shuttle.shuttle("security", m_security);
    shuttle.shuttle("name", m_name);
    shuttle.shuttle("security_view_stack", m_security_view_stack);
    shuttle.shuttle("identifier", m_identifier);
    shuttle.shuttle("link_identifier", m_link_identifier);
    shuttle.shuttle("geometry", m_geometry);
    shuttle.shuttle("bid_panel_header", m_bid_panel_header);
    shuttle.shuttle("ask_panel_header", m_ask_panel_header);
  }
}

namespace Beam {
  template<>
  constexpr auto
    shuttle_version<Spire::LegacyBookViewWindowSettings::Properties> = 2;
}

#endif
