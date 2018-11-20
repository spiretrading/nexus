#include "spire/book_view/book_view_table_widget.hpp"
#include "Nexus/Definitions/Side.hpp"
#include "spire/book_view/book_quote_table_model.hpp"
#include "spire/book_view/book_view_model.hpp"
#include "spire/book_view/book_view_side_table_view.hpp"
#include "spire/spire/dimensions.hpp"

using namespace Nexus;
using namespace Spire;

BookViewTableWidget::BookViewTableWidget(const BookViewModel& model,
    BookViewProperties properties, QWidget* parent)
    : QWidget(parent) {
  m_layout = new QHBoxLayout(this);
  m_layout->setContentsMargins({});
  m_layout->setSpacing(scale_width(2));
  m_bid_table_view = new BookViewSideTableView(
    std::make_unique<BookQuoteTableModel>(model, Side::BID, m_properties),
    this);
  m_layout->addWidget(m_bid_table_view);
  m_ask_table_view = new BookViewSideTableView(
    std::make_unique<BookQuoteTableModel>(model, Side::ASK, m_properties),
    this);
  m_layout->addWidget(m_ask_table_view);
  set_properties(std::move(properties));
}

void BookViewTableWidget::set_properties(BookViewProperties properties) {
  m_properties = std::move(properties);
  m_bid_table_view->set_properties(m_properties);
  m_ask_table_view->set_properties(m_properties);
}
