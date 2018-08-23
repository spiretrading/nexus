#ifndef SPIRE_BOOK_QUOTE_TABLE_MODEL_HPP
#define SPIRE_BOOK_QUOTE_TABLE_MODEL_HPP
#include <QAbstractTableModel>
#include "Nexus/Definitions/BookQuote.hpp"
#include "Nexus/Definitions/Market.hpp"
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

      //! Sets the properties that this model uses to style the table items.
      /*
        \param properties The properties to apply.
      */
      void set_properties(const BookViewProperties& properties);

    private:
      std::shared_ptr<BookViewModel> m_model;
      Nexus::Side m_side;
      BookViewProperties m_properties;
      std::vector<Nexus::BookQuote> m_data;
      std::unordered_map<Nexus::MarketCode , int>
        m_market_first_index;
      boost::signals2::scoped_connection m_book_quote_connection;

      void on_book_quote_signal();
  };
}

#endif
