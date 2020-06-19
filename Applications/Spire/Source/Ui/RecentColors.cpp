#include "Spire/Ui/RecentColors.hpp"

using namespace Spire;

const RecentColors& RecentColors::get_default_colors() {
  static auto default_colors = [] {
    auto colors = RecentColors();
    colors.add_color(Qt::white);
    colors.add_color(Qt::black);
    colors.add_color(Qt::darkGray);
    colors.add_color(Qt::magenta);
    colors.add_color(Qt::yellow);
    colors.add_color(Qt::cyan);
    colors.add_color(Qt::blue);
    colors.add_color(Qt::green);
    colors.add_color(Qt::red);
    return colors;
  }();
  return default_colors;
}

void RecentColors::add_color(const QColor& color) {
  m_recent_colors.push_front(color);
  if(m_recent_colors.size() > RECENT_COLOR_COUNT) {
    m_recent_colors.pop_back();
  }
}

std::array<QColor, RecentColors::RECENT_COLOR_COUNT>
    RecentColors::get_colors() const {
  auto colors = std::array<QColor, RECENT_COLOR_COUNT>();
  for(auto iter = m_recent_colors.begin(); iter != m_recent_colors.end();
      ++iter) {
    colors[std::distance(m_recent_colors.begin(), iter)] = *iter;
  }
  return colors;
}
