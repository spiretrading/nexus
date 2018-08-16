#include "spire/book_view/book_view_table_widget.hpp"
#include "spire/book_view/book_view_side_table_view.hpp"

using namespace Spire;

BookViewTableWidget::BookViewTableWidget(QWidget* parent)
    : QWidget(parent) {
  m_layout = new QHBoxLayout(this);
  m_layout->setContentsMargins({});
  m_layout->setSpacing(0);
  m_bid_table_view = new BookViewSideTableView(this);
  m_bid_table_view = new BookViewSideTableView(this);
}

void BookViewTableWidget::set_model(
    std::shared_ptr<BookQuoteTableModel> model) {
  m_bid_table_view->set_model(model);
  m_ask_table_view->set_model(model);
}

void BookViewTableWidget::set_properties(
    const BookViewProperties& properties) {
  m_bid_table_view->set_properties(properties);
  m_ask_table_view->set_properties(properties);
}
