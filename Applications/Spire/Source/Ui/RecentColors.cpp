#include "Spire/Ui/RecentColors.hpp"
#include <algorithm>

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
  auto iter = std::find_if(m_recent_colors.begin(), m_recent_colors.end(),
    [&] (const auto& element) { return element == color; });
  if(iter != m_recent_colors.end()) {
    m_recent_colors.erase(iter, std::min(iter + 1, m_recent_colors.end()));
  }
  m_recent_colors.push_front(color);
  if(m_recent_colors.size() > RECENT_COLOR_COUNT) {
    m_recent_colors.pop_back();
  }
}

std::array<QColor, RecentColors::RECENT_COLOR_COUNT>
    RecentColors::get_colors() const {
  auto colors = std::array<QColor, RECENT_COLOR_COUNT>();
  std::transform(m_recent_colors.begin(), m_recent_colors.end(),
    colors.begin(), [] (const auto& color) { return color; });
  return colors;
}

bool RecentColors::operator ==(const RecentColors& rhs) const {
  return m_recent_colors == rhs.m_recent_colors;
}

bool RecentColors::operator !=(const RecentColors& rhs) const {
  return !(*this == rhs);
}
