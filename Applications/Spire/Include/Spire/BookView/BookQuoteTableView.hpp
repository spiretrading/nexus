#ifndef SPIRE_BOOK_QUOTE_TABLE_VIEW_HPP
#define SPIRE_BOOK_QUOTE_TABLE_VIEW_HPP
#include <QTableView>
#include "Spire/BookView/BookView.hpp"

namespace Spire {

  //! Displays one side of a security's quotes in a table.
  class BookQuoteTableView : public QTableView {
    public:

      //! Constructs a BookQuoteTableView.
      /*
        \param model The model to get the table data from.
        \param parent The parent to this widget.
      */
      explicit BookQuoteTableView(std::unique_ptr<BookQuoteTableModel> model,
        QWidget* parent = nullptr);

      //! Sets the properties of the table.
      /*
        \param properties The properties the table will be updated to have.
      */
      void set_properties(const BookViewProperties& properties);

    protected:
      void resizeEvent(QResizeEvent* event) override;
      QStyleOptionViewItem viewOptions() const override;

    private:
      std::unique_ptr<BookQuoteTableModel> m_model;
  };
}

#endif
