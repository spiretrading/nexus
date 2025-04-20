#include "Spire/BookView/BookViewTableModel.hpp"
#include "Spire/Spire/ListToTableModel.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

using namespace boost;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Spire;

namespace {
  struct Extractor {
    AnyRef operator ()(const BookEntry& entry, int index) {
      auto column = static_cast<BookViewColumn>(index);
      if(column == BookViewColumn::MPID) {
        return entry;
      } else if(column == BookViewColumn::PRICE) {
        if(auto quote = get<BookQuote>(&entry)) {
          return quote->m_quote.m_price;
        } else if(auto order = get<BookViewModel::UserOrder>(&entry)) {
          return order->m_price;
        }
        return get<OrderFields>(entry).m_price;
      }
      if(auto quote = get<BookQuote>(&entry)) {
        return quote->m_quote.m_size;
      } else if(auto order = get<BookViewModel::UserOrder>(&entry)) {
        return order->m_size;
      }
      return get<OrderFields>(entry).m_quantity;
    }
  };

  const std::string& get_id(const BookEntry& entry) {
    if(auto quote = get<BookQuote>(&entry)) {
      return quote->m_mpid;
    } else if(auto order = get<BookViewModel::UserOrder>(&entry)) {
      return order->m_destination;
    }
    return get<OrderFields>(entry).m_destination;
  }
}

bool Spire::book_view_comparator(const AnyRef& left, const AnyRef& right) {
  if(left.get_type() == typeid(BookEntry)) {
    return get_id(any_cast<BookEntry>(left)) <
      get_id(any_cast<BookEntry>(right));
  }
  return Spire::compare(left, right);
}

std::shared_ptr<TableModel> Spire::make_book_view_table_model(
    std::shared_ptr<BookEntryListModel> entries) {
  return std::make_shared<ListToTableModel<BookEntry>>(
    std::move(entries), BOOK_VIEW_COLUMN_SIZE, Extractor());
}
