#include "Spire/TimeAndSales/TimeAndSalesWindowProperties.hpp"
#include "Spire/Spire/Dimensions.hpp"

using namespace Spire;

TimeAndSalesWindowProperties::TimeAndSalesWindowProperties()
    : m_font("Roboto"),
      m_show_grid(false) {
  m_font.setWeight(QFont::Medium);
  m_font.setPixelSize(scale_width(10));
  set_styles(BboIndicator::UNKNOWN, {QColor(Qt::black), QColor(0xFFFFFF)});
  set_styles(BboIndicator::ABOVE_ASK, {QColor(0x007735), QColor(0xEBFFF0)});
  set_styles(BboIndicator::AT_ASK, {QColor(0x007735), QColor(0xEBFFF0)});
  set_styles(BboIndicator::INSIDE, {QColor(Qt::black), QColor(0xFFFFFF)});
  set_styles(BboIndicator::AT_BID, {QColor(0xB71C1C), QColor(0xFFF1F1)});
  set_styles(BboIndicator::BELOW_BID, {QColor(0xB71C1C), QColor(0xFFF1F1)});
}

const TimeAndSalesWindowProperties::Styles&
    TimeAndSalesWindowProperties::get_styles(BboIndicator indicator) const {
  return m_styles[static_cast<int>(indicator)];
}

void TimeAndSalesWindowProperties::set_styles(BboIndicator indicator,
    const Styles& style) {
  m_styles[static_cast<int>(indicator)] = style;
}

const QFont& TimeAndSalesWindowProperties::get_font() const {
  return m_font;
}

void TimeAndSalesWindowProperties::set_font(const QFont& font) {
  m_font = font;
}

bool TimeAndSalesWindowProperties::is_show_grid() const {
  return m_show_grid;
}

void TimeAndSalesWindowProperties::set_show_grid(bool show_grid) {
  m_show_grid = show_grid;
}
