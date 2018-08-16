#ifndef SPIRE_BOOK_QUOTE_TABLE_MODEL_HPP
#define SPIRE_BOOK_QUOTE_TABLE_MODEL_HPP
#include <QAbstractTableModel>
#include "Nexus/Definitions/BookQuote.hpp"
#include "Nexus/Definitions/Side.hpp"
#include "spire/book_view/book_view.hpp"
#include "spire/book_view/book_view_properties.hpp"

namespace Spire {

  //! Implements the table model for one side of book quotes in a list view.
  class BookQuoteTableModel : public QAbstractTableModel {
    public:

      //! Constructs a BookQuoteTableModel.
      BookQuoteTableModel(std::shared_ptr<BookViewModel> model,
        const Nexus::Side& side, const BookViewProperties& properties);

      int rowCount(const QModelIndex& parent) const override;

      int columnCount(const QModelIndex& parent) const override;

      QVariant data(const QModelIndex& index, int role) const override;

    private:
      std::shared_ptr<BookViewModel> m_model;
      Nexus::Side m_side;
      BookViewProperties m_properties;
      std::vector<Nexus::BookQuote> m_data;
  };
}

#endif
