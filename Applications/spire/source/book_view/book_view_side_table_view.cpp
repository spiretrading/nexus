#include "spire/book_view/book_view_side_table_view.hpp"
#include <QHeaderView>
#include "spire/book_view/book_quote_table_model.hpp"

using namespace Spire;

BookViewSideTableView::BookViewSideTableView(QWidget* parent)
    : QTableView(parent) {
  horizontalHeader()->hide();
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  verticalHeader()->hide();
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setSelectionMode(QAbstractItemView::NoSelection);
  setFocusPolicy(Qt::NoFocus);
  setShowGrid(false);
}

void BookViewSideTableView::set_model(
    std::unique_ptr<BookQuoteTableModel> model) {
  m_model = std::move(model);
  setModel(m_model.get());
}

void BookViewSideTableView::set_properties(
    const BookViewProperties& properties) {
  m_model->set_properties(properties);
}
