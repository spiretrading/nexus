#ifndef SPIRE_TIME_AND_SALES_PROPERTIES_HPP
#define SPIRE_TIME_AND_SALES_PROPERTIES_HPP
#include <array>
#include <QColor>
#include <QFont>
#include "Spire/TimeAndSales/TimeAndSales.hpp"

namespace Spire {

  //! Stores the properties used to display a time and sales window.
  struct TimeAndSalesProperties {

    //! The position of a time and sale print relative to the BBO price.
    enum class PriceRange {

      //! The BBO price was not known.
      UNKNOWN,

      //! The time and sale price was greater than the BBO ask.
      ABOVE_ASK,

      //! The time and sale price was equal to the BBO ask.
      AT_ASK,

      //! The time and sale price is inbetween the BBO.
      INSIDE,

      //! The time and sale price is equal to the BBO bid.
      AT_BID,

      //! The time and sale price is less than the BBO bid.
      BELOW_BID
    };

    //! The number of enumerated price ranges.
    static const auto PRICE_RANGE_COUNT = 6;

    //! The available columns to display.
    enum class Column {

      //! The time column.
      TIME_COLUMN,

      //! The price column.
      PRICE_COLUMN,

      //! The size column.
      SIZE_COLUMN,

      //! The market column.
      MARKET_COLUMN,

      //! The sales condition column.
      CONDITION_COLUMN
    };

    //! The number of columns in this model.
    static const auto COLUMN_COUNT = 5;

    //! The list of text colors used.
    std::array<QColor, PRICE_RANGE_COUNT> m_text_colors;

    //! The list of band colors used.
    std::array<QColor, PRICE_RANGE_COUNT> m_band_colors;

    //! Whether to display a grid.
    bool m_show_grid;

    //! The font used.
    QFont m_font;

    //! Constructs a default set of properties.
    TimeAndSalesProperties();

    //! Returns a text color.
    const QColor& get_text_color(PriceRange index) const noexcept;

    //! Sets a text color.
    void set_text_color(PriceRange index, const QColor& color) noexcept;

    //! Returns a band color.
    const QColor& get_band_color(PriceRange index) const noexcept;

    //! Sets a band color.
    void set_band_color(PriceRange index, const QColor& color) noexcept;
  };
}

#endif
