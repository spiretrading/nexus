#include "Spire/TimeAndSales/TimeAndSalesWindowProperties.hpp"
#include "Spire/Spire/Dimensions.hpp"

using namespace Spire;

TimeAndSalesWindowProperties::TimeAndSalesWindowProperties()
    : m_show_grid(false) {
  auto font = QFont("Roboto");
  font.setWeight(QFont::Medium);
  font.setPixelSize(scale_height(10));
  set_style(BboIndicator::UNKNOWN, {QColor(Qt::black), QColor(0xFFFFFF), font});
  set_style(BboIndicator::ABOVE_ASK,
    {QColor(0x007735), QColor(0xEBFFF0), font});
  set_style(BboIndicator::AT_ASK, {QColor(0x007735), QColor(0xEBFFF0), font});
  set_style(BboIndicator::INSIDE, {QColor(Qt::black), QColor(0xFFFFFF), font});
  set_style(BboIndicator::AT_BID, {QColor(0xB71C1C), QColor(0xFFF1F1), font});
  set_style(BboIndicator::BELOW_BID,
    {QColor(0xB71C1C), QColor(0xFFF1F1), font});
}

const TimeAndSalesWindowProperties::Styles&
    TimeAndSalesWindowProperties::get_style(BboIndicator indicator) const {
  return m_styles[static_cast<int>(indicator)];
}

void TimeAndSalesWindowProperties::set_style(BboIndicator indicator,
    const Styles& style) {
  m_styles[static_cast<int>(indicator)] = style;
}

bool TimeAndSalesWindowProperties::is_show_grid() const {
  return m_show_grid;
}

void TimeAndSalesWindowProperties::set_show_grid(bool show_grid) {
  m_show_grid = show_grid;
}
