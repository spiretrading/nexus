#ifndef SPIRE_RECENT_COLORS_HPP
#define SPIRE_RECENT_COLORS_HPP
#include <array>
#include <deque>
#include <QColor>

namespace Spire {

  class RecentColors {
    public:

      static const int RECENT_COLOR_COUNT = 9;

      static const RecentColors& get_default_colors();

      RecentColors() = default;

      void add_color(const QColor& color);

      std::array<QColor, RECENT_COLOR_COUNT> get_colors() const;

      bool operator ==(const RecentColors& rhs) const;

      bool operator !=(const RecentColors& rhs) const;

    private:
      std::deque<QColor> m_recent_colors;
  };
}

#endif
