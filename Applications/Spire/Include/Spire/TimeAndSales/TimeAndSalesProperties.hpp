#ifndef SPIRE_TIME_AND_SALES_PROPERTIES_HPP
#define SPIRE_TIME_AND_SALES_PROPERTIES_HPP
#include <array>
#include <QColor>
#include <QFont>
#include "Spire/TimeAndSales/BboIndicator.hpp"
#include "Spire/TimeAndSales/TimeAndSales.hpp"
#include "Spire/Ui/HighlightBox.hpp"

namespace Spire {

  /** Represents the properties used to in the time and sales window. */
  class TimeAndSalesProperties {
    public:

      using Highlight = HighlightColor;

      /* Constructs default properties. */
      TimeAndSalesProperties();

      /* Returns the highlight of a specific BBO indicator. */
      const Highlight& get_highlight(BboIndicator indicator) const;

      /* Sets the highlight to a specific BBO indicator. */
      void set_highlight(BboIndicator indicator, const Highlight& highlight);
    
      /* Returns the font. */
      const QFont& get_font() const;
    
      /* Sets the font. */
      void set_font(const QFont& font);
    
      /** Returns <code>true</code> iff the grid is shown. */
      bool is_show_grid() const;

      /** Sets whether to show the grid. */
      void set_show_grid(bool show_grid);

    private:
      std::array<Highlight, BBO_INDICATOR_COUNT> m_highlights;
      QFont m_font;
      bool m_show_grid;
  };
}

#endif
