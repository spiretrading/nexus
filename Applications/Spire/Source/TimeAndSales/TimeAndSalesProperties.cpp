#include "Spire/TimeAndSales/TimeAndSalesProperties.hpp"

using namespace Spire;

TimeAndSalesProperties::TimeAndSalesProperties() {
  set_text_color(PriceRange::UNKNOWN, QColor(0, 0, 0));
  set_band_color(PriceRange::UNKNOWN, QColor(243, 243, 243));
  set_text_color(PriceRange::ABOVE_ASK, QColor(0, 119, 53));
  set_band_color(PriceRange::ABOVE_ASK, QColor(196, 250, 194));
  set_text_color(PriceRange::AT_ASK, QColor(0, 119, 53));
  set_band_color(PriceRange::AT_ASK, QColor(196, 250, 194));
  set_text_color(PriceRange::INSIDE, QColor(0, 0, 0));
  set_band_color(PriceRange::INSIDE, QColor(243, 243, 243));
  set_text_color(PriceRange::AT_BID, QColor(197, 0, 0));
  set_band_color(PriceRange::AT_BID, QColor(225, 220, 220));
  set_text_color(PriceRange::BELOW_BID, QColor(197, 0, 0));
  set_band_color(PriceRange::BELOW_BID, QColor(225, 220, 220));
  m_show_grid = false;
  m_font = QFont("Roboto", 8, QFont::Medium);
}

const QColor& TimeAndSalesProperties::get_text_color(
    PriceRange index) const noexcept {
  return m_text_colors[static_cast<int>(index)];
}

void TimeAndSalesProperties::set_text_color(PriceRange index,
    const QColor& color) noexcept {
  m_text_colors[static_cast<int>(index)] = color;
}

const QColor& TimeAndSalesProperties::get_band_color(
    PriceRange index) const noexcept {
  return m_band_colors[static_cast<int>(index)];
}

void TimeAndSalesProperties::set_band_color(PriceRange index,
    const QColor& color) noexcept {
  m_band_colors[static_cast<int>(index)] = color;
}
