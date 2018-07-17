#include "spire/book_view/market_list_item.hpp"

using namespace spire;
using Entry = Nexus::MarketDatabase::Entry;

market_list_item::market_list_item(const Entry& entry, QListWidget* parent)
    : QListWidgetItem(entry.m_code.GetData(), parent),
      m_market(entry) {}
