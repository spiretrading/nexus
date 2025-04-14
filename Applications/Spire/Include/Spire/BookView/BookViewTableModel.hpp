#ifndef SPIRE_BOOK_VIEW_TABLE_MODEL_HPP
#define SPIRE_BOOK_VIEW_TABLE_MODEL_HPP
#include <string>
#include <boost/variant/variant.hpp>
#include "Nexus/Definitions/BookQuote.hpp"
#include "Nexus/OrderExecutionService/OrderFields.hpp"
#include "Spire/BookView/BookView.hpp"
#include "Spire/BookView/BookViewModel.hpp"
#include "Spire/Spire/AnyRef.hpp"
#include "Spire/Spire/TableModel.hpp"
#include "Spire/Spire/ListModel.hpp"

namespace Spire {

  /**
   * Stores one of the values displayed as a row in the TableView for a book,
   * namely a book quote representing market data, an order submitted by a user,
   * or an order being previewed for submission.
   */
  using BookEntry = boost::variant<Nexus::BookQuote, BookViewModel::UserOrder,
    Nexus::OrderExecutionService::OrderFields>;

  /** The type used for a list of BookEntries. */
  using BookEntryListModel = ListModel<BookEntry>;

  /* Enumerates the columns of the BookViewTableModel. */
  enum class BookViewColumn {

    /** The MPID column. */
    MPID,

    /** The price column. */
    PRICE,

    /** The size column. */
    SIZE
  };

  /** The number of columns in a TableModel representing a BookView. */
  static const auto BOOK_VIEW_COLUMN_SIZE = 3;

  /** Represents a MPID column and where it originated. */
  struct Mpid {

    /** Specifies where the MPID originated. */
    enum class Origin {

      /** The MPID is from a BookQuote. */
      BOOK_QUOTE,

      /** The MPID is from a user submitted order. */
      USER_ORDER,

      /** The MPID is from an order preview. */
      PREVIEW
    };

    /** The identifier to display. */
    std::string m_id;

    /** The MPID's originating market. */
    Nexus::MarketCode m_market;

    /** The source of the listing. */
    Origin m_origin;

    /** Compares two Mpids by id. */
    bool operator <(const Mpid& mpid) const;
  };

  /** Implements the comparator used by the BookEntry TableModel. */
  bool book_view_comparator(const AnyRef& left, const AnyRef& right);

  /** Makes a TableModel as a view over a list of BookEntries. */
  std::shared_ptr<TableModel> make_book_view_table_model(
    std::shared_ptr<BookEntryListModel> entries);
}

#endif
