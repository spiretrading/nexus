#ifndef SPIRE_BOOK_VIEW_PROPERTIES_HPP
#define SPIRE_BOOK_VIEW_PROPERTIES_HPP
#include <unordered_map>
#include <vector>
#include <Beam/Pointers/Out.hpp>
#include <Beam/Serialization/ShuttleOptional.hpp>
#include <Beam/Serialization/ShuttleVector.hpp>
#include <boost/optional/optional.hpp>
#include <QColor>
#include <QFont>
#include "Nexus/Definitions/Market.hpp"
#include "Spire/BookView/BookView.hpp"
#include "Spire/LegacyUI/ShuttleQtTypes.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /** Stores the properties used by a BookViewWindow. */
  class BookViewProperties {
    public:

      /** Stores a market's highlight properties. */
      struct MarketHighlight {

        /** The color to highlight the market with. */
        QColor m_color;

        /** Whether to highlight all levels. */
        bool m_highlightAllLevels;

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

      /** Returns the default BookViewProperties. */
      static BookViewProperties GetDefault();

      /**
       * Loads the BookViewWindow Properties from a UserProfile.
       * @param userProfile The UserProfile to load the properties from.
       */
      static void Load(Beam::Out<UserProfile> userProfile);

      /**
       * Saves a UserProfile's BookViewWindow Properties.
       * @param userProfile The UserProfile's properties to save.
       */
      static void Save(const UserProfile& userProfile);

      /** Constructs an uninitialized BookViewProperties. */
      BookViewProperties();

      /** Returns the foreground color of a BookQuote. */
      const QColor& GetBookQuoteForegroundColor() const;

      /** Sets the foreground color of a BookQuote. */
      void SetBookQuoteForegroundColor(const QColor& color);

      /**
       * Returns the list of background colors used for each level of a
       * BookQuote.
       */
      const std::vector<QColor>& GetBookQuoteBackgroundColors() const;

      /**
       * Returns the list of background colors used for each level of a
       * BookQuote.
       */
      std::vector<QColor>& GetBookQuoteBackgroundColors();

      /** Returns the font used for the BboQuote. */
      const QFont& GetBboQuoteFont() const;

      /** Sets the font used for the BboQuote. */
      void SetBboQuoteFont(const QFont& font);

      /** Returns the font to used by BookQuotes. */
      const QFont& GetBookQuoteFont() const;

      /** Sets the font used by BookQuotes. */
      void SetBookQuoteFont(const QFont& font);

      /**
       * Returns the MarketHighlight for a specified market.
       * @param market The market to get the property for.
       * @return The <i>market</i>'s highlight property.
       */
      boost::optional<const MarketHighlight&> GetMarketHighlight(
        Nexus::MarketCode market) const;

      /**
       * Sets the MarketHighlight for a specified market.
       * @param market The market to apply the property to.
       * @param highlight The MarketHighlight to apply.
       */
      void SetMarketHighlight(
        Nexus::MarketCode market, const MarketHighlight& highlight);

      /**
       * Removes the MarketHighlight for a specified market.
       * @param market The market to remove the highlight from.
       */
      void RemoveMarketHighlight(Nexus::MarketCode market);

      /** Returns the OrderHighlight option. */
      OrderHighlight GetOrderHighlight() const;

      /** Sets the OrderHighlight option. */
      void SetOrderHighlight(OrderHighlight orderHighlight);

      /** Returns the OrderHighlight color. */
      const QColor& GetOrderHighlightColor() const;

      /** Sets the OrderHighlight color. */
      void SetOrderHighlightColor(const QColor& color);

      /** Returns <code>true</code> iff grid lines should be displayed. */
      bool GetShowGrid() const;

      /** Sets whether grid lines should be displayed. */
      void SetShowGrid(bool value);

      /** Returns <code>true</code> iff the BBO should be displayed. */
      bool GetShowBbo() const;

      /** Sets whether the BBO should be displayed. */
      void SetShowBbo(bool value);

    private:
      friend struct Beam::Serialization::DataShuttle;
      QColor m_bookQuoteForegroundColor;
      std::vector<QColor> m_bookQuoteBackgroundColors;
      QFont m_bboQuoteFont;
      QFont m_bookQuoteFont;
      std::unordered_map<Nexus::MarketCode, MarketHighlight> m_marketHighlights;
      OrderHighlight m_orderHighlight;
      QColor m_orderHighlightColor;
      bool m_showGrid;
      bool m_showBbo;

      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void BookViewProperties::MarketHighlight::Shuttle(
      Shuttler& shuttle, unsigned int version) {
    shuttle.Shuttle("color", m_color);
    shuttle.Shuttle("highlight_all_levels", m_highlightAllLevels);
  }

  template<typename Shuttler>
  void BookViewProperties::Shuttle(Shuttler& shuttle, unsigned int version) {
    shuttle.Shuttle("book_quote_foreground_color", m_bookQuoteForegroundColor);
    shuttle.Shuttle(
      "book_quote_background_colors", m_bookQuoteBackgroundColors);
    shuttle.Shuttle("bbo_quote_font", m_bboQuoteFont);
    shuttle.Shuttle("book_quote_font", m_bookQuoteFont);
    shuttle.Shuttle("market_highlights", m_marketHighlights);
    shuttle.Shuttle("order_highlight", m_orderHighlight);
    shuttle.Shuttle("order_highlight_color", m_orderHighlightColor);
    shuttle.Shuttle("show_grid", m_showGrid);
    if(version >= 2) {
      shuttle.Shuttle("show_bbo", m_showBbo);
    } else {
      m_showBbo = false;
    }
  }
}

namespace Beam::Serialization {
  template<>
  struct Version<Spire::BookViewProperties> :
    std::integral_constant<unsigned int, 2> {};
}

#endif
