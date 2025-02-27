#ifndef SPIRE_BOOK_VIEW_TABLE_MODEL_HPP
#define SPIRE_BOOK_VIEW_TABLE_MODEL_HPP
#include "Nexus/Definitions/BookQuote.hpp"
#include "Spire/BookView/BookView.hpp"
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

  /**
   * Returns mpid for the specified row in the quote table.
   * @param table The quote table.
   * @param row The index of the row.
   */
  const std::string& get_mpid(const TableModel& table, int row);

  /**
   * Returns the price for the specified row in the quote table.
   * @param table The quote table.
   * @param row The index of the row.
   */
  const Nexus::Money& get_price(const TableModel& table, int row);

  /**
   * Returns the size for the specified row in the quote table.
   * @param table The quote table.
   * @param row The index of the row.
   */
  const Nexus::Quantity& get_size(const TableModel& table, int row);

  /** Returns <code>true</code> when mpid is an order. */
  bool is_order(const std::string& mpid);
}

#endif
