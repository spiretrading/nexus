#ifndef SPIRE_BOOK_VIEW_TABLE_MODEL_HPP
#define SPIRE_BOOK_VIEW_TABLE_MODEL_HPP
#include "Nexus/Definitions/BookQuote.hpp"
#include "Spire/Spire/TableModel.hpp"

namespace Spire {

  /* Enumerates the columns of the BookViewTableModel. */
  enum class BookViewColumns {

    /** The MPID column. */
    MPID,

    /** The price column. */
    PRICE,

    /** The size column. */
    SIZE
  };

  /** Makes a TableModel as a view over a ListModel<BookQuote>. */
  std::shared_ptr<TableModel> make_book_view_table_model(
    std::shared_ptr<ListModel<Nexus::BookQuote>> book_quotes);
}

#endif
