#ifndef SPIRE_BOOK_VIEW_SIDE_TABLE_VIEW_HPP
#define SPIRE_BOOK_VIEW_SIDE_TABLE_VIEW_HPP
#include <QTableView>
#include "spire/book_view/book_view.hpp"

namespace Spire {

  //! Displays one side of a security's quotes in a table.
  class BookViewSideTableView : public QTableView {
    public:

      //! Constructs a BookViewSideTableView.
      /*
        \param model The model to get the table data from.
        \param parent The parent to this widget.
      */
      explicit BookViewSideTableView(std::unique_ptr<BookQuoteTableModel> model,
        QWidget* parent = nullptr);

      //! Sets the properties of the table.
      /*
        \param properties The properties the table will be updated to have.
      */
      void set_properties(const BookViewProperties& properties);

    protected:
      void resizeEvent(QResizeEvent* event) override;

    private:
      std::unique_ptr<BookQuoteTableModel> m_model;
  };
}

#endif
