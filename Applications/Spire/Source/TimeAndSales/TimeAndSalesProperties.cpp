#include "Spire/TimeAndSales/TimeAndSalesProperties.hpp"
#include "Spire/Spire/Dimensions.hpp"

using namespace Spire;

const TimeAndSalesProperties& TimeAndSalesProperties::get_default() {
  static auto PROPERTIES = [] {
    auto properties = TimeAndSalesProperties();
    properties.set_highlight_color(BboIndicator::UNKNOWN,
      {QColor(0xFFFFFF), QColor(Qt::black)});
    properties.set_highlight_color(BboIndicator::ABOVE_ASK,
      {QColor(0xEBFFF0), QColor(0x007735)});
    properties.set_highlight_color(BboIndicator::AT_ASK,
      {QColor(0xEBFFF0), QColor(0x007735)});
    properties.set_highlight_color(BboIndicator::INSIDE,
      {QColor(0xFFFFFF), QColor(Qt::black)});
    properties.set_highlight_color(BboIndicator::AT_BID,
      {QColor(0xFFF1F1), QColor(0xB71C1C)});
    properties.set_highlight_color(BboIndicator::BELOW_BID,
      {QColor(0xFFF1F1), QColor(0xB71C1C)});
    auto font = QFont("Roboto");
    font.setWeight(QFont::Medium);
    font.setPixelSize(scale_width(10));
    properties.set_font(std::move(font));
    properties.set_grid_enabled(false);
    return properties;
  }();
  return PROPERTIES;
}

const HighlightColor& TimeAndSalesProperties::get_highlight_color(
    BboIndicator indicator) const {
  return m_highlight_colors[static_cast<int>(indicator)];
}

void TimeAndSalesProperties::set_highlight_color(BboIndicator indicator,
    const HighlightColor& highlight_color) {
  m_highlight_colors[static_cast<int>(indicator)] = highlight_color;
}

const QFont& TimeAndSalesProperties::get_font() const {
  return m_font;
}

void TimeAndSalesProperties::set_font(const QFont& font) {
  m_font = font;
}

bool TimeAndSalesProperties::is_grid_enabled() const {
  return m_is_grid_enabled;
}

void TimeAndSalesProperties::set_grid_enabled(bool is_enabled) {
  m_is_grid_enabled = is_enabled;
}
