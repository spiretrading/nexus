#include "Spire/BookView/BookViewTableModel.hpp"
#include "Spire/Spire/ListToTableModel.hpp"

using namespace Nexus;
using namespace Spire;

namespace {
  static const auto COLUMN_SIZE = 3;

  AnyRef extract(const BookQuote& quote, int index) {
    if(index == 0) {
      return quote.m_mpid;
    } else if(index == 1) {
      return quote.m_quote.m_price;
    }
    return quote.m_quote.m_size;
  }
}

std::shared_ptr<TableModel> Spire::make_book_view_table_model(
    std::shared_ptr<ListModel<BookQuote>> book_quotes) {
  return std::make_shared<ListToTableModel<BookQuote>>(
    std::move(book_quotes), COLUMN_SIZE, &extract);
}
