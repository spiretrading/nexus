#include "spire/book_view/book_view_properties.hpp"

using namespace boost;
using namespace Nexus;
using namespace spire;

book_view_properties::book_view_properties() {
  set_bbo_quote_font(QFont("Roboto", 14, QFont::Medium));
  set_book_quote_font(QFont("Roboto", 8, QFont::Medium));
  QColor baseColor(75, 35, 160);
  for(auto i = 0; i < 7; ++i) {
    get_book_quote_background_colors().push_back(
      baseColor.lighter(300 - i * (200 / 7)));
  }
  set_book_quote_foreground_color(QColor(0, 0, 0));
  set_order_highlight_color(QColor(60, 225, 83));
  set_show_grid(false);
}

const QColor& book_view_properties::get_book_quote_foreground_color() const {
  return m_book_quote_foreground_color;
}

void book_view_properties::set_book_quote_foreground_color(
    const QColor& color) {
  m_book_quote_foreground_color = color;
}

const std::vector<QColor>&
    book_view_properties::get_book_quote_background_colors() const {
  return m_book_quote_background_colors;
}

std::vector<QColor>& book_view_properties::get_book_quote_background_colors() {
  return m_book_quote_background_colors;
}

const QFont& book_view_properties::get_bbo_quote_font() const {
  return m_bbo_quote_font;
}

void book_view_properties::set_bbo_quote_font(const QFont& font) {
  m_bbo_quote_font = font;
}

const QFont& book_view_properties::get_book_quote_font() const {
  return m_book_quote_font;
}

void book_view_properties::set_book_quote_font(const QFont& font) {
  m_book_quote_font = font;
}

optional<const book_view_properties::market_highlight&>
    book_view_properties::get_market_highlight(MarketCode market) const {
  auto i = m_market_highlights.find(market);
  if(i == m_market_highlights.end()) {
    return none;
  }
  return i->second;
}

void book_view_properties::set_market_highlight(MarketCode market,
    const market_highlight& highlight) {
  m_market_highlights[market] = highlight;
}

void book_view_properties::remove_market_highlight(MarketCode market) {
  m_market_highlights.erase(market);
}

book_view_properties::order_highlight
    book_view_properties::get_order_highlight() const {
  return m_order_highlight;
}

void book_view_properties::set_order_highlight(order_highlight highlight) {
  m_order_highlight = highlight;
}

const QColor& book_view_properties::get_order_highlight_color() const {
  return m_order_highlight_color;
}

void book_view_properties::set_order_highlight_color(const QColor& color) {
  m_order_highlight_color = color;
}

bool book_view_properties::get_show_grid() const {
  return m_show_grid;
}

void book_view_properties::set_show_grid(bool value) {
  m_show_grid = value;
}
