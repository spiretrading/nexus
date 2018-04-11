#ifndef SPIRE_TIME_AND_SALES_PROPERTIES_HPP
#define SPIRE_TIME_AND_SALES_PROPERTIES_HPP
#include <array>
#include <QColor>
#include <QFont>
#include "spire/time_and_sales/time_and_sales.hpp"
#include "spire/time_and_sales/time_and_sales_model.hpp"

namespace spire {

  //! Stores the properties used to display a time and sales window.
  struct time_and_sales_properties {

    //! The number of enumerated price ranges.
    static const auto PRICE_RANGE_COUNT = 6;

    //! The number of enumerated columns.
    static const auto COLUMN_COUNT = 5;

    //! The list of text colors used.
    std::array<QColor, PRICE_RANGE_COUNT> m_text_colors;

    //! The list of band colors used.
    std::array<QColor, PRICE_RANGE_COUNT> m_band_colors;

    //! The list of columns to show.
    std::array<bool, COLUMN_COUNT> m_show_columns;

    //! Whether to display a grid.
    bool m_show_grid;

    //! The font used.
    QFont m_font;

    //! Constructs a default set of properties.
    time_and_sales_properties();

    //! Returns a text color.
    const QColor& get_text_color(
      time_and_sales_model::price_range index) const noexcept;

    //! Sets a text color.
    void set_text_color(time_and_sales_model::price_range index,
      const QColor& color) noexcept;

    //! Returns a band color.
    const QColor& get_band_color(
      time_and_sales_model::price_range index) const noexcept;

    //! Sets a band color.
    void set_band_color(time_and_sales_model::price_range index,
      const QColor& color) noexcept;

    //! Returns the visibility of a column.
    bool get_show_column(time_and_sales_model::columns column) const noexcept;

    //! Sets the visibility of a column.
    void set_show_column(time_and_sales_model::columns column,
      bool show) noexcept;
  };
}

#endif
