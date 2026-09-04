#include "Spire/BookView/BookViewTableModel.hpp"
#include "Spire/Spire/ListToTableModel.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

using namespace boost;
using namespace Nexus;
using namespace Spire;

namespace {
  const std::string& get_id(const BookEntry& entry) {
    if(auto quote = get<BookQuote>(&entry)) {
      return quote->m_mpid;
    } else if(auto order = get<BookViewModel::UserOrder>(&entry)) {
      return order->m_destination;
    }
    return get<OrderFields>(entry).m_destination;
  }

  const Money& get_price(const BookEntry& entry) {
    if(auto quote = get<BookQuote>(&entry)) {
      return quote->m_quote.m_price;
    } else if(auto order = get<BookViewModel::UserOrder>(&entry)) {
      return order->m_price;
    }
    return get<OrderFields>(entry).m_price;
  }

  const Quantity& get_size(const BookEntry& entry) {
    if(auto quote = get<BookQuote>(&entry)) {
      return quote->m_quote.m_size;
    } else if(auto order = get<BookViewModel::UserOrder>(&entry)) {
      return order->m_size;
    }
    return get<OrderFields>(entry).m_quantity;
  }

  struct Extractor {
    AnyRef operator ()(const BookEntry& entry, int index) {
      auto column = static_cast<BookViewColumn>(index);
      if(column == BookViewColumn::MPID) {
        return entry;
      } else if(column == BookViewColumn::PRICE) {
        return get_price(entry);
      }
      return get_size(entry);
    }
  };
}

bool Spire::book_view_comparator(const AnyRef& left, const AnyRef& right) {
  if(left.get_type() == typeid(Money)) {
    return any_cast<Money>(left) < any_cast<Money>(right);
  } else if(left.get_type() == typeid(Quantity)) {
    return any_cast<Quantity>(left) < any_cast<Quantity>(right);
  } else if(left.get_type() == typeid(BookEntry)) {
    return get_id(any_cast<BookEntry>(left)) <
      get_id(any_cast<BookEntry>(right));
  }
  return compare(left, right);
}

std::shared_ptr<TableModel> Spire::make_book_view_table_model(
    std::shared_ptr<BookEntryListModel> entries) {
  return std::make_shared<ListToTableModel<BookEntry>>(
    std::move(entries), BOOK_VIEW_COLUMN_SIZE, Extractor());
}
