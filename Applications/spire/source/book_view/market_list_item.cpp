#include "spire/book_view/market_list_item.hpp"
#include "spire/book_view/book_view_properties.hpp"

using namespace boost;
using namespace spire;
using Entry = Nexus::MarketDatabase::Entry;

market_list_item::market_list_item(const Entry& entry, QListWidget* parent)
    : QListWidgetItem(entry.m_code.GetData(), parent),
      m_market(entry) {}

const Entry& market_list_item::get_market_info() const {
  return m_market;
}

optional<const book_view_properties::market_highlight&>
    market_list_item::get_market_highlight() const {
  return m_market_highlight;
}
