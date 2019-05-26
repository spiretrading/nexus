#ifndef SPIRE_BOOK_VIEW_TABLE_VIEW_HPP
#define SPIRE_BOOK_VIEW_TABLE_VIEW_HPP
#include <QHBoxLayout>
#include <QWidget>
#include "spire/book_view/book_view.hpp"
#include "spire/book_view/book_view_properties.hpp"

namespace Spire {

  //! Displays both sides of a security's book quotes.
  class BookViewTableWidget : public QWidget {
    public:
      //! Constructs a BookViewTableWidget.
      /*
        \param model The model to get the table data from.
        \param properties The properties the table will be updated to have.
        \param parent The parent to this widget.
      */
      explicit BookViewTableWidget(const BookViewModel& model,
        BookViewProperties properties, QWidget* parent = nullptr);

      //! Sets the properties of the table.
      /*
        \param properties The properties the table will be updated to have.
      */
      void set_properties(BookViewProperties properties);

    private:
      QHBoxLayout* m_layout;
      BookQuoteTableView* m_bid_table_view;
      BookQuoteTableView* m_ask_table_view;
      BookViewProperties m_properties;
  };
}

#endif
