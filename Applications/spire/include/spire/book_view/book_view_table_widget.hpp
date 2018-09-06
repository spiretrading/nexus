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
        \param parent The parent to this widget.
      */
      BookViewTableWidget(QWidget* parent = nullptr);

      //! Sets the model for this table widget.
      /*
        \param model The model to get the table data from.
      */
      void set_model(std::shared_ptr<BookViewModel> model);

      //! Sets the properties of the table.
      /*
        \param properties The properties the table will be updated to have.
      */
      void set_properties(const BookViewProperties& properties);

    private:
      QHBoxLayout* m_layout;
      BookViewSideTableView* m_bid_table_view;
      BookViewSideTableView* m_ask_table_view;
      BookViewProperties m_properties;
  };
}

#endif
