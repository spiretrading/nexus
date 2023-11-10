#include "Spire/TimeAndSales/TimeAndSalesProperties.hpp"
#include <QFontDatabase>
#include "Spire/Spire/Dimensions.hpp"

using namespace Spire;

TimeAndSalesProperties::TimeAndSalesProperties()
    : m_font(QFontDatabase().font("Roboto", "Medium", -1)),
      m_show_grid(false) {
  m_font.setPixelSize(scale_width(10));
  set_highlight(BboIndicator::UNKNOWN, {QColor(0xFFFFFF), QColor(Qt::black)});
  set_highlight(BboIndicator::ABOVE_ASK, {QColor(0xEBFFF0), QColor(0x007735)});
  set_highlight(BboIndicator::AT_ASK, {QColor(0xEBFFF0), QColor(0x007735)});
  set_highlight(BboIndicator::INSIDE, {QColor(0xFFFFFF), QColor(Qt::black)});
  set_highlight(BboIndicator::AT_BID, {QColor(0xFFF1F1), QColor(0xB71C1C)});
  set_highlight(BboIndicator::BELOW_BID, {QColor(0xFFF1F1), QColor(0xB71C1C)});
}

const TimeAndSalesProperties::Highlight&
    TimeAndSalesProperties::get_highlight(BboIndicator indicator) const {
  return m_highlights[static_cast<int>(indicator)];
}

void TimeAndSalesProperties::set_highlight(BboIndicator indicator,
    const Highlight& style) {
  m_highlights[static_cast<int>(indicator)] = style;
}

const QFont& TimeAndSalesProperties::get_font() const {
  return m_font;
}

void TimeAndSalesProperties::set_font(const QFont& font) {
  m_font = font;
}

bool TimeAndSalesProperties::is_show_grid() const {
  return m_show_grid;
}

void TimeAndSalesProperties::set_show_grid(bool show_grid) {
  m_show_grid = show_grid;
}
