#include "spire/book_view/book_view_table_widget.hpp"
#include "Nexus/Definitions/Side.hpp"
#include "spire/book_view/book_quote_table_model.hpp"
#include "spire/book_view/book_view_model.hpp"
#include "spire/book_view/book_view_side_table_view.hpp"

using namespace Nexus;
using namespace Spire;

BookViewTableWidget::BookViewTableWidget(QWidget* parent)
    : QWidget(parent) {
  m_layout = new QHBoxLayout(this);
  m_layout->setContentsMargins({});
  m_layout->setSpacing(0);
  m_bid_table_view = new BookViewSideTableView(this);
  m_layout->addWidget(m_bid_table_view);
  m_ask_table_view = new BookViewSideTableView(this);
  m_layout->addWidget(m_ask_table_view);
}

void BookViewTableWidget::set_model(std::shared_ptr<BookViewModel> model) {
  m_bid_table_view->set_model(std::make_unique<BookQuoteTableModel>(
    model, Side::BID, m_properties));
  m_ask_table_view->set_model(std::make_unique<BookQuoteTableModel>(
    model, Side::ASK, m_properties));
}

void BookViewTableWidget::set_properties(
    const BookViewProperties& properties) {
  m_properties = properties;
  m_bid_table_view->set_properties(m_properties);
  m_ask_table_view->set_properties(m_properties);
}
