#ifndef SPIRE_BOOK_QUOTE_TABLE_MODEL_HPP
#define SPIRE_BOOK_QUOTE_TABLE_MODEL_HPP
#include <QAbstractTableModel>
#include "Nexus/Definitions/Market.hpp"
#include "Nexus/Definitions/Side.hpp"
#include "Spire/BookView/BookView.hpp"
#include "Spire/BookView/BookViewModel.hpp"
#include "Spire/BookView/BookViewProperties.hpp"
#include "Spire/Spire/Signal.hpp"

namespace Spire {

  //! Implements the table model for one side of book quotes in a list view.
  class BookQuoteTableModel : public QAbstractTableModel {
    public:

      //! Constructs a BookQuoteTableModel.
      /*!
        \param model The model to represent.
        \param side The side to model.
        \param properties The window's display properties.
      */
      BookQuoteTableModel(const BookViewModel& model, Nexus::Side side,
        const BookViewProperties& properties);

      int rowCount(const QModelIndex& parent) const override;

      int columnCount(const QModelIndex& parent) const override;

      QVariant data(const QModelIndex& index, int role) const override;

      //! Sets the properties that this model uses to style the table items.
      /*!
        \param properties The properties to apply.
      */
      void set_properties(const BookViewProperties& properties);

    private:
      const BookViewModel* m_model;
      Nexus::Side m_side;
      BookViewProperties m_properties;
      std::unordered_map<Nexus::MarketCode, int> m_first_market_index;
      int m_size;
      boost::signals2::scoped_connection m_quote_connection;

      void on_quote_signal(const BookViewModel::Quote& quote, int index);
      void update_first_market_indexes(int index);
  };
}

#endif
