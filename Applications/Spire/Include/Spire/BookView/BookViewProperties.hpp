#ifndef SPIRE_BOOK_VIEW_PROPERTIES_HPP
#define SPIRE_BOOK_VIEW_PROPERTIES_HPP
#include <vector>
#include <QColor>
#include <QFont>

namespace Spire {

  /** Represents the price level properties in the book view window. */
  struct BookViewLevelProperties {

    /** The font used in the text. */
    QFont m_font;

    /** Whether to show grid lines. */
    bool m_is_grid_enabled;

    /** A list of color values used to designate distinct price levels. */
    std::vector<QColor> m_color_scheme;

    /** Returns the default properties. */
    static const BookViewLevelProperties& get_default();
  };
}

#endif
