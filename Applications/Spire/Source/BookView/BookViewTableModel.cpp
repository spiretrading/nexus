#include "Spire/BookView/BookViewTableModel.hpp"
#include "Spire/Spire/ListToTableModel.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

using namespace boost;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Spire;

namespace {
  struct Extractor {
    Mpid m_mpid;

    AnyRef operator ()(const BookEntry& entry, int index) {
      auto column = static_cast<BookViewColumn>(index);
      if(auto quote = get<BookQuote>(&entry)) {
        if(column == BookViewColumn::MPID) {
          m_mpid = Mpid(Mpid::Origin::BOOK_QUOTE, quote->m_mpid);
          return std::as_const(m_mpid);
        } else if(column == BookViewColumn::PRICE) {
          return quote->m_quote.m_price;
        }
        return quote->m_quote.m_size;
      } else if(auto order = get<BookViewModel::UserOrder>(&entry)) {
        if(column == BookViewColumn::MPID) {
          m_mpid = Mpid(Mpid::Origin::USER_ORDER, order->m_destination);
          return std::as_const(m_mpid);
        } else if(column == BookViewColumn::PRICE) {
          return order->m_price;
        }
        return order->m_size;
      }
      auto preview = get<OrderFields>(&entry);
      if(column == BookViewColumn::MPID) {
        m_mpid = Mpid(Mpid::Origin::PREVIEW, preview->m_destination);
        return std::as_const(m_mpid);
      } else if(column == BookViewColumn::PRICE) {
        return preview->m_price;
      }
      return preview->m_quantity;
    }
  };
}

bool Spire::Mpid::operator <(const Mpid& mpid) const {
  return m_id < mpid.m_id;
}

bool Spire::book_view_comparator(const AnyRef& left, const AnyRef& right) {
  if(left.get_type() == typeid(Mpid)) {
    return any_cast<Mpid>(left) < any_cast<Mpid>(right);
  }
  return Spire::compare(left, right);
}

std::shared_ptr<TableModel> Spire::make_book_view_table_model(
    std::shared_ptr<BookEntryListModel> entries) {
  return std::make_shared<ListToTableModel<BookEntry>>(
    std::move(entries), BOOK_VIEW_COLUMN_SIZE, Extractor());
}
