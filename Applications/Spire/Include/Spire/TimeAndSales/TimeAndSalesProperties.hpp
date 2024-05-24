#ifndef SPIRE_TIME_AND_SALES_PROPERTIES_HPP
#define SPIRE_TIME_AND_SALES_PROPERTIES_HPP
#include <array>
#include <QFont>
#include "Spire/TimeAndSales/BboIndicator.hpp"
#include "Spire/TimeAndSales/TimeAndSales.hpp"
#include "Spire/Ui/HighlightBox.hpp"

namespace Spire {

  /** Represents the properties used in the time and sales window. */
  class TimeAndSalesProperties {
    public:

      using Highlight = HighlightColor;

      /* Constructs default properties. */
      TimeAndSalesProperties();

      /**
       * Returns the highlight of a specific BBO indicator.
       * @param indicator The highlighted indicator.
       */
      const Highlight& get_highlight(BboIndicator indicator) const;

      /**
       * Sets the highlight to a specific BBO indicator.
       * @param indicator The indicator to be highlighted.
       * @param highlight The highlight to be applied to the indicator.
       */
      void set_highlight(BboIndicator indicator, const Highlight& highlight);

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
      std::array<Highlight, BBO_INDICATOR_COUNT> m_highlights;
      QFont m_font;
      bool m_is_grid_enabled;
  };
}

#endif
