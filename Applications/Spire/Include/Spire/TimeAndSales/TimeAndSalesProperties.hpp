#ifndef SPIRE_TIME_AND_SALES_PROPERTIES_HPP
#define SPIRE_TIME_AND_SALES_PROPERTIES_HPP
#include <array>
#include <bitset>
#include <filesystem>
#include <Beam/Serialization/ShuttleArray.hpp>
#include <Beam/Serialization/ShuttleBitset.hpp>
#include <QFont>
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Spire/ShuttleQtTypes.hpp"
#include "Spire/TimeAndSales/BboIndicator.hpp"
#include "Spire/TimeAndSales/TimeAndSalesTableModel.hpp"
#include "Spire/Ui/HighlightBox.hpp"

namespace Spire {

  /** Represents the properties used in the time and sales window. */
  class TimeAndSalesProperties {
    public:

      /** Returns the default properties. */
      static const TimeAndSalesProperties& get_default();

       /* Constructs an empty set of properties. */
      TimeAndSalesProperties();

      /**
       * Returns the highlight color of a specific BBO indicator.
       * @param indicator The highlighted indicator.
       */
      const HighlightColor& get_highlight_color(BboIndicator indicator) const;

      /**
       * Sets the highlight color to a specific BBO indicator.
       * @param indicator The indicator to be highlighted.
       * @param highlight_color The highlight color of the indicator.
       */
      void set_highlight_color(
        BboIndicator indicator, const HighlightColor& highlight_color);

      /* Returns the font. */
      const QFont& get_font() const;

      /**
       * Sets the font used in the time and sales window.
       * @param font The font to be used.
       */
      void set_font(const QFont& font);

      /** Returns <code>true</code> iff a column is visible. */
      bool is_visible(TimeAndSalesTableModel::Column column) const;

      /** Sets whether a column is visible. */
      void set_visible(TimeAndSalesTableModel::Column column, bool is_visible);

      /** Returns <code>true</code> iff the grid is shown. */
      bool is_grid_enabled() const;

      /**
       * Sets whether to enable display grid.
       * @param is_enabled True iff the grid is shown.
       */
      void set_grid_enabled(bool is_enabled);

      /* Returns the column order. */
      const std::array<int, TimeAndSalesTableModel::COLUMN_SIZE>&
        get_column_order() const;

      /**
       * Moves a column from source to destination.
       * @param source The index of the column to move.
       * @param destination The index to move the column to.
       */
      void move_column(TimeAndSalesTableModel::Column source,
        TimeAndSalesTableModel::Column destination);

    private:
      friend struct Beam::DataShuttle;
      std::array<HighlightColor, BBO_INDICATOR_COUNT> m_highlight_colors;
      QFont m_font;
      std::array<int, TimeAndSalesTableModel::COLUMN_SIZE> m_column_order;
      std::bitset<TimeAndSalesTableModel::COLUMN_SIZE> m_visible_columns;
      bool m_is_grid_enabled;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  /** A ValueModel over a TimeAndSalesProperties. */
  using TimeAndSalesPropertiesModel = ValueModel<TimeAndSalesProperties>;

  /** A LocalValueModel over a TimeAndSalesProperties. */
  using LocalTimeAndSalesPropertiesModel =
    LocalValueModel<TimeAndSalesProperties>;

  /**
   * Loads TimeAndSalesProperties from a file.
   * @param path The path to the file to load.
   * @return The properties loaded from the given <i>path</i>.
   */
  TimeAndSalesProperties load_time_and_sales_properties(
    const std::filesystem::path& path);

  /**
   * Saves TimeAndSalesProperties to a file.
   * @param properties The properties to save.
   * @param path The path to the save the properties to.
   */
  void save_time_and_sales_properties(const TimeAndSalesProperties& properties,
    const std::filesystem::path& path);

  template<Beam::IsShuttle S>
  void TimeAndSalesProperties::shuttle(S& shuttle, unsigned int version) {
    shuttle.shuttle("highlight_colors", m_highlight_colors);
    shuttle.shuttle("font", m_font);
    shuttle.shuttle("visible_columns", m_visible_columns);
    shuttle.shuttle("is_grid_enabled", m_is_grid_enabled);
    shuttle.shuttle("column_order", m_column_order);
  }
}

#endif
