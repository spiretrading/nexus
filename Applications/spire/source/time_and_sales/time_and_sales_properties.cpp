#include "spire/time_and_sales/time_and_sales_properties.hpp"

using namespace spire;

time_and_sales_properties::time_and_sales_properties() {
  set_text_color(price_range::UNKNOWN, QColor(0, 0, 0));
  set_band_color(price_range::UNKNOWN, QColor(243, 243, 243));
  set_text_color(price_range::ABOVE_ASK, QColor(0, 119, 53));
  set_band_color(price_range::ABOVE_ASK, QColor(196, 250, 194));
  set_text_color(price_range::AT_ASK, QColor(0, 119, 53));
  set_band_color(price_range::AT_ASK, QColor(196, 250, 194));
  set_text_color(price_range::INSIDE, QColor(0, 0, 0));
  set_band_color(price_range::INSIDE, QColor(243, 243, 243));
  set_text_color(price_range::AT_BID, QColor(197, 0, 0));
  set_band_color(price_range::AT_BID, QColor(225, 220, 220));
  set_text_color(price_range::BELOW_BID, QColor(197, 0, 0));
  set_band_color(price_range::BELOW_BID, QColor(225, 220, 220));
  m_show_grid = false;
  m_font = QFont("Roboto", 8, QFont::Medium);
}

const QColor& time_and_sales_properties::get_text_color(
    price_range index) const noexcept {
  return m_text_colors[static_cast<int>(index)];
}

void time_and_sales_properties::set_text_color(price_range index,
    const QColor& color) noexcept {
  m_text_colors[static_cast<int>(index)] = color;
}

const QColor& time_and_sales_properties::get_band_color(
    price_range index) const noexcept {
  return m_band_colors[static_cast<int>(index)];
}

void time_and_sales_properties::set_band_color(price_range index,
    const QColor& color) noexcept {
  m_band_colors[static_cast<int>(index)] = color;
}
