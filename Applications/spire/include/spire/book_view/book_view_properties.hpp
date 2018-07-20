#ifndef SPIRE_BOOK_VIEW_PROPERTIES_HPP
#define SPIRE_BOOK_VIEW_PROPERTIES_HPP
#include <unordered_map>
#include <vector>
#include <boost/optional/optional.hpp>
#include <QColor>
#include <QFont>
#include "Nexus/Definitions/Market.hpp"
#include "spire/book_view/book_view.hpp"

namespace spire {

  //! Stores the properties used to display a book_view_window.
  class book_view_properties {
    public:

      //! Stores a market's highlight properties.
      struct market_highlight {

        //! The color to highlight the market with.
        QColor m_color;

        //! Whether to highlight all levels.
        bool m_highlight_all_levels;
      };

      //! Stores options available to highlight Orders.
      enum class order_highlight {

        //! Do not display Orders.
        HIDE_ORDERS,

        //! Display Orders using the BookQuote colors.
        DISPLAY_ORDERS,

        //! Display Orders using a specified color.
        HIGHLIGHT_ORDERS
      };

      //! Constructs the default properties.
      book_view_properties();

      //! Returns the foreground color of a BookQuote.
      const QColor& get_book_quote_foreground_color() const;

      //! Sets the foreground color of a BookQuote.
      void set_book_quote_foreground_color(const QColor& color);

      //! Returns the list of background colors used for each level of a
      //! BookQuote.
      const std::vector<QColor>& get_book_quote_background_colors() const;

      //! Returns the list of background colors used for each level of a
      //! BookQuote.
      std::vector<QColor>& get_book_quote_background_colors();

      //! Returns the font used for the BboQuote.
      const QFont& get_bbo_quote_font() const;

      //! Sets the font used for the BboQuote.
      void set_bbo_quote_font(const QFont& font);

      //! Returns the font to used by BookQuotes.
      const QFont& get_book_quote_font() const;

      //! Sets the font used by BookQuotes.
      void set_book_quote_font(const QFont& font);

      //! Returns the market_highlight for a specified market.
      /*!
        \param market The market to get the property for.
        \return The <i>market</i>'s highlight property.
      */
      boost::optional<const market_highlight&> get_market_highlight(
        Nexus::MarketCode market) const;

      //! Sets the market_highlight for a specified market.
      /*!
        \param market The market to apply the property to.
        \param highlight The market_highlight to apply.
      */
      void set_market_highlight(Nexus::MarketCode market,
        const market_highlight& highlight);

      //! Removes the market_highlight for a specified market.
      /*!
        \param market The market to remove the highlight from.
      */
      void remove_market_highlight(Nexus::MarketCode market);

      //! Returns the order_highlight option.
      order_highlight get_order_highlight() const;

      //! Sets the order_highlight option.
      void set_order_highlight(order_highlight highlight);

      //! Returns the order_highlight color.
      const QColor& get_order_highlight_color() const;

      //! Sets the order_highlight color.
      void set_order_highlight_color(const QColor& color);

      //! Returns <code>true</code> iff grid lines should be displayed.
      bool get_show_grid() const;

      //! Sets whether grid lines should be displayed.
      void set_show_grid(bool value);

    private:
      QColor m_book_quote_foreground_color;
      std::vector<QColor> m_book_quote_background_colors;
      QFont m_bbo_quote_font;
      QFont m_book_quote_font;
      std::unordered_map<Nexus::MarketCode, market_highlight>
        m_market_highlights;
      order_highlight m_order_highlight;
      QColor m_order_highlight_color;
      bool m_show_grid;
  };
}

#endif
