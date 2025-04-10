#ifndef SPIRE_BOOK_VIEW_TABLE_MODEL_HPP
#define SPIRE_BOOK_VIEW_TABLE_MODEL_HPP
#include "Spire/BookView/BookView.hpp"
#include "Spire/BookView/MergedBookQuoteListModel.hpp"
#include "Spire/Spire/TableModel.hpp"

namespace Spire {

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

  /** Specifies where a book listing originated from. */
  enum class ListingSource {

    /** The listing is from a BookQuote. */
    BOOK_QUOTE,

    /** The listing is from a user submitted order. */
    USER_ORDER,

    /** The listing is from a preview. */
    PREVIEW
  };

  /** Represents a listing's MPID column and it's source. */
  struct MpidListing {

    /** The source of the listing. */
    ListingSource m_source;

    /** The MPID to display for this listing. */
    std::string m_mpid;
  };

  /** Makes a TableModel as a view over a ListModel<BookListing>. */
  std::shared_ptr<TableModel> make_book_view_table_model(
    std::shared_ptr<ListModel<BookListing>> book_quotes);
}

#endif
