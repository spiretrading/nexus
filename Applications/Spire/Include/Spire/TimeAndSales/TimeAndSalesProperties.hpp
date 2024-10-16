#ifndef SPIRE_TIME_AND_SALES_PROPERTIES_HPP
#define SPIRE_TIME_AND_SALES_PROPERTIES_HPP
#include <array>
#include <filesystem>
#include <Beam/Serialization/ShuttleArray.hpp>
#include <QFont>
#include "Spire/Spire/ShuttleQtTypes.hpp"
#include "Spire/TimeAndSales/BboIndicator.hpp"
#include "Spire/TimeAndSales/TimeAndSales.hpp"
#include "Spire/Ui/HighlightBox.hpp"

namespace Spire {

  /** Represents the properties used in the time and sales window. */
  class TimeAndSalesProperties {
    public:

      /** Returns the default properties. */
      static const TimeAndSalesProperties& get_default();

       /* Constructs an empty set of properties. */
      TimeAndSalesProperties() = default;

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
      void set_highlight_color(BboIndicator indicator,
        const HighlightColor& highlight_color);

      /* Returns the font. */
      const QFont& get_font() const;

      /**
       * Sets the font used in the time and sales window.
       * @param font The font to be used.
       */
      void set_font(const QFont& font);

      /** Returns <code>true</code> iff the grid is shown. */
      bool is_grid_enabled() const;

      /**
       * Sets whether to enable display grid.
       * @param is_enabled True iff the grid is shown.
       */
      void set_grid_enabled(bool is_enabled);

    private:
      friend struct Beam::Serialization::DataShuttle;
      std::array<HighlightColor, BBO_INDICATOR_COUNT> m_highlight_colors;
      QFont m_font;
      bool m_is_grid_enabled;

      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

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

  template<typename Shuttler>
  void TimeAndSalesProperties::Shuttle(
      Shuttler& shuttle, unsigned int version) {
    shuttle.Shuttle("highlight_colors", m_highlight_colors);
    shuttle.Shuttle("font", m_font);
    shuttle.Shuttle("is_grid_enabled", m_is_grid_enabled);
  }
}

#endif
