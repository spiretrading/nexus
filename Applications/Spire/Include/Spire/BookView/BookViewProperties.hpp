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
#include "Nexus/Definitions/Venue.hpp"
#include "Spire/BookView/BookView.hpp"
#include "Spire/Spire/ShuttleQtTypes.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {
  class UserProfile;

  /** Stores the properties used by a BookViewWindow. */
  class BookViewProperties {
    public:

      /** Stores a venue's highlight properties. */
      struct VenueHighlight {

        /** The color to highlight the venue with. */
        QColor m_color;

        /** Whether to highlight all levels. */
        bool m_highlightAllLevels;

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
       * Returns the VenueHighlight for a specified venue.
       * @param venue The venue to get the property for.
       * @return The <i>venue</i>'s highlight property.
       */
      boost::optional<const VenueHighlight&> GetVenueHighlight(
        Nexus::Venue venue) const;

      /**
       * Sets the VenueHighlight for a specified venue.
       * @param venue The venue to apply the property to.
       * @param highlight The VenueHighlight to apply.
       */
      void SetVenueHighlight(
        Nexus::Venue venue, const VenueHighlight& highlight);

      /**
       * Removes the VenueHighlight for a specified venue.
       * @param venue The venue to remove the highlight from.
       */
      void RemoveVenueHighlight(Nexus::Venue venue);

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
      friend struct Beam::DataShuttle;
      QColor m_bookQuoteForegroundColor;
      std::vector<QColor> m_bookQuoteBackgroundColors;
      QFont m_bboQuoteFont;
      QFont m_bookQuoteFont;
      std::unordered_map<Nexus::Venue, VenueHighlight> m_venueHighlights;
      OrderHighlight m_orderHighlight;
      QColor m_orderHighlightColor;
      bool m_showGrid;
      bool m_showBbo;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void BookViewProperties::VenueHighlight::shuttle(
      S& shuttle, unsigned int version) {
    shuttle.shuttle("color", m_color);
    shuttle.shuttle("highlight_all_levels", m_highlightAllLevels);
  }

  template<Beam::IsShuttle S>
  void BookViewProperties::shuttle(S& shuttle, unsigned int version) {
    shuttle.shuttle("book_quote_foreground_color", m_bookQuoteForegroundColor);
    shuttle.shuttle(
      "book_quote_background_colors", m_bookQuoteBackgroundColors);
    shuttle.shuttle("bbo_quote_font", m_bboQuoteFont);
    shuttle.shuttle("book_quote_font", m_bookQuoteFont);
    shuttle.shuttle("venue_highlights", m_venueHighlights);
    shuttle.shuttle("order_highlight", m_orderHighlight);
    shuttle.shuttle("order_highlight_color", m_orderHighlightColor);
    shuttle.shuttle("show_grid", m_showGrid);
    if(version >= 2) {
      shuttle.shuttle("show_bbo", m_showBbo);
    } else {
      m_showBbo = false;
    }
  }
}

namespace Beam {
  template<>
  constexpr unsigned int shuttle_version<Spire::BookViewProperties> = 2;
}

#endif
