#include "spire/book_view/market_list_item.hpp"

using namespace boost;
using namespace spire;
using Entry = Nexus::MarketDatabase::Entry;

market_list_item::market_list_item(const Entry& entry, QListWidget* parent)
    : QListWidgetItem(entry.m_code.GetData(), parent),
      m_market(entry) {}

const Entry& market_list_item::get_market_info() const {
  return m_market;
}

const optional<book_view_properties::market_highlight>&
    market_list_item::get_market_highlight() const {
  return m_market_highlight;
}

void market_list_item::set_highlight_color(const QColor& color) {
  if(!m_market_highlight.is_initialized()) {
    initialize();
    m_market_highlight->m_highlight_all_levels = false;
  }
  m_market_highlight->m_color = color;
}

void market_list_item::set_highlight_all_levels() {
  if(!m_market_highlight.is_initialized()) {
    initialize();
  }
  m_market_highlight->m_highlight_all_levels = true;
}

void market_list_item::set_highlight_top_level() {
  if(!m_market_highlight.is_initialized()) {
    initialize();
  }
  m_market_highlight->m_highlight_all_levels = false;
}

void market_list_item::remove_highlight() {
  m_market_highlight = none;
}

void market_list_item::initialize() {
  m_market_highlight.emplace();
  m_market_highlight->m_color = Qt::white;
}
