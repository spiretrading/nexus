#include "Spire/Ui/RecentColors.hpp"
#include <algorithm>

using namespace boost::signals2;
using namespace Spire;

RecentColors::RecentColors() {
  m_recent_colors.push_front(Qt::white);
  m_recent_colors.push_front(Qt::black);
  m_recent_colors.push_front(Qt::darkGray);
  m_recent_colors.push_front(Qt::magenta);
  m_recent_colors.push_front(Qt::yellow);
  m_recent_colors.push_front(Qt::cyan);
  m_recent_colors.push_front(Qt::blue);
  m_recent_colors.push_front(Qt::green);
  m_recent_colors.push_front(Qt::red);
}

RecentColors& RecentColors::get_instance() {
  static auto current_colors = RecentColors();
  return current_colors;
}

void RecentColors::add_color(const QColor& color) {
  if(m_recent_colors.front() == color) {
    return;
  }
  auto iter = std::find(m_recent_colors.begin(), m_recent_colors.end(), color);
  if(iter != m_recent_colors.end()) {
    m_recent_colors.erase(iter, iter + 1);
  }
  m_recent_colors.push_front(color);
  if(m_recent_colors.size() > RECENT_COLOR_COUNT) {
    m_recent_colors.pop_back();
  }
  m_change_signal();
}

std::array<QColor, RecentColors::RECENT_COLOR_COUNT>
    RecentColors::get_colors() const {
  auto colors = std::array<QColor, RECENT_COLOR_COUNT>();
  std::copy(m_recent_colors.begin(), m_recent_colors.end(),
    colors.begin());
  return colors;
}

connection RecentColors::connect_change_signal(
    const ChangeSignal::slot_type& slot) const {
  return m_change_signal.connect(slot);
}
