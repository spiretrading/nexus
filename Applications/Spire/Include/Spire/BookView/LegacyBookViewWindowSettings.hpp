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

        /** Stores a market's highlight properties. */
        struct MarketHighlight {

          /** The color to highlight the market with. */
          QColor m_color;

          /** Whether to highlight all levels. */
          bool m_highlight_all_levels;

          template<typename Shuttler>
          void Shuttle(Shuttler& shuttle, unsigned int version);
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

        /** Associates with each market code a highlight. */
        std::unordered_map<Nexus::MarketCode, MarketHighlight>
          m_market_highlights;

        /** The OrderHighlight option. */
        OrderHighlight m_order_highlight;

        /** The OrderHighlight color. */
        QColor m_order_highlight_color;

        /** <code>true</code> iff grid lines should be displayed. */
        bool m_show_grid;

        /** <code>true</code> iff the BBO should be displayed. */
        bool m_show_bbo;

        template<typename Shuttler>
        void Shuttle(Shuttler& shuttle, unsigned int version);
      };

      /** Constructs a LegacyBookViewWindowSettings with default values. */
      LegacyBookViewWindowSettings() = default;

      std::string GetName() const override;

      QWidget* Reopen(Beam::Ref<UserProfile> userProfile) const override;

      void Apply(Beam::Ref<UserProfile> userProfile,
        Beam::Out<QWidget> widget) const override;

    private:
      friend struct Beam::Serialization::DataShuttle;
      Properties m_properties;
      Nexus::Security m_security;
      std::string m_name;
      LegacyUI::SecurityViewStack m_security_view_stack;
      std::string m_identifier;
      std::string m_link_identifier;
      QByteArray m_geometry;
      QByteArray m_bid_panel_header;
      QByteArray m_ask_panel_header;

      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void LegacyBookViewWindowSettings::Properties::MarketHighlight::Shuttle(
      Shuttler& shuttle, unsigned int version) {
    shuttle.Shuttle("color", m_color);
    shuttle.Shuttle("highlight_all_levels", m_highlight_all_levels);
  }

  template<typename Shuttler>
  void LegacyBookViewWindowSettings::Properties::Shuttle(
      Shuttler& shuttle, unsigned int version) {
    shuttle.Shuttle(
      "book_quote_foreground_color", m_book_quote_foreground_color);
    shuttle.Shuttle(
      "book_quote_background_colors", m_book_quote_background_colors);
    shuttle.Shuttle("bbo_quote_font", m_bbo_quote_font);
    shuttle.Shuttle("book_quote_font", m_book_quote_font);
    shuttle.Shuttle("market_highlights", m_market_highlights);
    shuttle.Shuttle("order_highlight", m_order_highlight);
    shuttle.Shuttle("order_highlight_color", m_order_highlight_color);
    shuttle.Shuttle("show_grid", m_show_grid);
    if(version >= 2) {
      shuttle.Shuttle("show_bbo", m_show_bbo);
    } else {
      m_show_bbo = false;
    }
  }

  template<typename Shuttler>
  void LegacyBookViewWindowSettings::Shuttle(
      Shuttler& shuttle, unsigned int version) {
    shuttle.Shuttle("properties", m_properties);
    shuttle.Shuttle("security", m_security);
    shuttle.Shuttle("name", m_name);
    shuttle.Shuttle("security_view_stack", m_security_view_stack);
    shuttle.Shuttle("identifier", m_identifier);
    shuttle.Shuttle("link_identifier", m_link_identifier);
    shuttle.Shuttle("geometry", m_geometry);
    shuttle.Shuttle("bid_panel_header", m_bid_panel_header);
    shuttle.Shuttle("ask_panel_header", m_ask_panel_header);
  }
}

namespace Beam::Serialization {
  template<>
  struct Version<Spire::LegacyBookViewWindowSettings::Properties> :
    std::integral_constant<unsigned int, 2> {};
}

#endif
