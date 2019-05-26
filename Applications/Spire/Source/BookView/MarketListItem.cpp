#include "spire/book_view/market_list_item.hpp"

using namespace boost;
using namespace Spire;
using Entry = Nexus::MarketDatabase::Entry;

MarketListItem::MarketListItem(const Entry& entry, QListWidget* parent)
    : QListWidgetItem(entry.m_code.GetData(), parent),
      m_market(entry) {}

const Entry& MarketListItem::get_market_info() const {
  return m_market;
}

const optional<BookViewProperties::MarketHighlight>&
    MarketListItem::get_market_highlight() const {
  return m_market_highlight;
}

void MarketListItem::set_highlight_color(const QColor& color) {
  if(!m_market_highlight.is_initialized()) {
    initialize();
    m_market_highlight->m_highlight_all_levels = false;
  }
  m_market_highlight->m_color = color;
}

void MarketListItem::set_highlight_all_levels() {
  if(!m_market_highlight.is_initialized()) {
    initialize();
  }
  m_market_highlight->m_highlight_all_levels = true;
}

void MarketListItem::set_highlight_top_level() {
  if(!m_market_highlight.is_initialized()) {
    initialize();
  }
  m_market_highlight->m_highlight_all_levels = false;
}

void MarketListItem::remove_highlight() {
  m_market_highlight = none;
}

void MarketListItem::initialize() {
  m_market_highlight.emplace();
  m_market_highlight->m_color = Qt::white;
}
