#ifndef SPIRE_TIMEANDSALESPROPERTIES_HPP
#define SPIRE_TIMEANDSALESPROPERTIES_HPP
#include <array>
#include <Beam/Pointers/Out.hpp>
#include <Beam/Serialization/ShuttleArray.hpp>
#include <QColor>
#include <QFont>
#include "Spire/Spire/ShuttleQtTypes.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/TimeAndSales/TimeAndSales.hpp"

namespace Spire {
  class UserProfile;

  /*! \class TimeAndSalesProperties
      \brief Stores the properties used by a TimeAndSalesWindow.
    */
  class TimeAndSalesProperties {
    public:

      //! The number of enumerated price ranges.
      static const unsigned int PRICE_RANGE_COUNT = 6;

      //! The number of enumerated columns.
      static const unsigned int COLUMN_COUNT = 5;

      //! Returns the default TimeAndSalesProperties.
      static TimeAndSalesProperties GetDefault();

      //! Loads the TimeAndSalesProperties from a UserProfile.
      /*!
        \param userProfile The UserProfile storing the properties.
      */
      static void Load(Beam::Out<UserProfile> userProfile);

      //! Saves a UserProfile's TimeAndSalesProperties.
      /*!
        \param userProfile The UserProfile's properties to save.
      */
      static void Save(const UserProfile& userProfile);

      //! Constructs an uninitialized TimeAndSalesProperties.
      TimeAndSalesProperties();

      //! Returns the foreground colors used for the price ranges.
      const std::array<QColor, PRICE_RANGE_COUNT>&
        GetPriceRangeForegroundColor() const;

      //! Returns the foreground colors used for the price ranges.
      std::array<QColor, PRICE_RANGE_COUNT>& GetPriceRangeForegroundColor();

      //! Returns the background colors used for the price ranges.
      const std::array<QColor, PRICE_RANGE_COUNT>&
        GetPriceRangeBackgroundColor() const;

      //! Returns the background colors used for the price ranges.
      std::array<QColor, PRICE_RANGE_COUNT>& GetPriceRangeBackgroundColor();

      //! Returns the visible columns.
      const std::array<bool, COLUMN_COUNT>& GetVisibleColumns() const;

      //! Returns the visible columns.
      std::array<bool, COLUMN_COUNT>& GetVisibleColumns();

      //! Returns whether the grid lines are visible.
      bool GetShowGridLines() const;

      //! Sets whether to show the grid lines.
      void SetShowGridLines(bool value);

      //! Returns whether the vertical scrollbar is visible.
      bool IsVerticalScrollBarVisible() const;

      //! Sets whether the vertical scrollbar is visible.
      void SetVerticalScrollBarVisible(bool isVisible);

      //! Returns whether the horizontal scrollbar is visible.
      bool IsHorizontalScrollBarVisible() const;

      //! Sets whether the horizontal scrollbar is visible.
      void SetHorizontalScrollBarVisible(bool isVisible);

      //! Returns the display font.
      const QFont& GetFont() const;

      //! Sets the display font.
      void SetFont(const QFont& font);

    private:
      friend struct Beam::DataShuttle;
      std::array<QColor, PRICE_RANGE_COUNT> m_priceRangeForegroundColor;
      std::array<QColor, PRICE_RANGE_COUNT> m_priceRangeBackgroundColor;
      std::array<bool, COLUMN_COUNT> m_visibleColumns;
      bool m_showGridLines;
      bool m_verticalScrollBarVisible;
      bool m_horizontalScrollBarVisible;
      QFont m_font;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void TimeAndSalesProperties::shuttle(S& shuttle, unsigned int version) {
    shuttle.shuttle("price_range_foreground_color",
      m_priceRangeForegroundColor);
    shuttle.shuttle("price_range_background_color",
      m_priceRangeBackgroundColor);
    shuttle.shuttle("visible_columns", m_visibleColumns);
    shuttle.shuttle("show_grid_lines", m_showGridLines);
    shuttle.shuttle("vertical_scroll_bar_visible", m_verticalScrollBarVisible);
    shuttle.shuttle("horizontal_scroll_bar_visible",
      m_horizontalScrollBarVisible);
    shuttle.shuttle("font", m_font);
  }
}

#endif
