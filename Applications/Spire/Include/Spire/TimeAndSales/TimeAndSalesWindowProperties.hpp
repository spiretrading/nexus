#ifndef SPIRE_TIME_AND_SALES_WINDOW_PROPERTIES_HPP
#define SPIRE_TIME_AND_SALES_WINDOW_PROPERTIES_HPP
#include <array>
#include <QColor>
#include <QFont>
#include "Spire/TimeAndSales/BboIndicator.hpp"
#include "Spire/TimeAndSales/TimeAndSales.hpp"

namespace Spire {

  /** Represents the properties used to in the time and sales window. */
  class TimeAndSalesWindowProperties {
    public:

      /* The style properties of the BBO indicator. */
      struct Styles {

        /* The text color. */
        QColor m_text_color;

        /* The band color. */
        QColor m_band_color;
      };

      /* Constructs default properties. */
      TimeAndSalesWindowProperties();

      /* Returns the styles of the BBO indicator. */
      const Styles& get_styles(BboIndicator indicator) const;

      /* Sets the styles to a BBO indicator. */
      void set_styles(BboIndicator indicator, const Styles& styles);

      /* Returns the font. */
      const QFont& get_font() const;

      /* Sets the font. */
      void set_font(const QFont& font);

      /** Returns <code>true</code> iff the grid is shown. */
      bool is_show_grid() const;

      /** Sets whether to show the grid. */
      void set_show_grid(bool show_grid);

    private:
      std::array<Styles, BBO_INDICATOR_COUNT> m_styles;
      QFont m_font;
      bool m_show_grid;
  };
}

#endif