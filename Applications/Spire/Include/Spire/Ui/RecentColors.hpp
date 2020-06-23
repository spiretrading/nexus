#ifndef SPIRE_RECENT_COLORS_HPP
#define SPIRE_RECENT_COLORS_HPP
#include <array>
#include <deque>
#include <QColor>

namespace Spire {

  //! Represents a data structure for storing recently selected colors in a
  //  FIFO manner.
  class RecentColors {
    public:

      //! The maximum number of recent colors to store.
      static const int RECENT_COLOR_COUNT = 9;

      //! Returns the default recent colors.
      static const RecentColors& get_default_colors();

      RecentColors() = default;

      //! Inserts a color into the recently selected colors.
      /*!
        \param color The color to insert.
      */
      void add_color(const QColor& color);

      //! Returns the collection of recently selected colors.
      std::array<QColor, RECENT_COLOR_COUNT> get_colors() const;

      //! Returns true if two RecentColors share the same recently selected
      //! colors in the same order.
      bool operator ==(const RecentColors& rhs) const;

      //! Returns false if two RecentColors do not share the same recently
      //! selected colors in the same order.
      bool operator !=(const RecentColors& rhs) const;

    private:
      std::deque<QColor> m_recent_colors;
  };
}

#endif
