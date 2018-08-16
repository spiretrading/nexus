#ifndef SPIRE_BOOK_QUOTE_TABLE_MODEL_HPP
#define SPIRE_BOOK_QUOTE_TABLE_MODEL_HPP
#include <QAbstractTableModel>
#include "Nexus/Definitions/Side.hpp"
#include "spire/book_view/book_view.hpp"

namespace Spire {

  //! Implements the table model for one side of book quotes in a list view.
  class BookQuoteTableModel : public QAbstractTableModel {
    public:

      //! Constructs a BookQuoteTableModel.
      BookQuoteTableModel(std::shared_ptr<BookViewModel> model,
        const Nexus::Side& side, const BookViewProperties& properties);
  };
}

#endif
