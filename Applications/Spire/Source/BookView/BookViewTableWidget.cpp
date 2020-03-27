#include "Spire/BookView/BookViewTableWidget.hpp"
#include "Nexus/Definitions/Side.hpp"
#include "Spire/BookView/BookQuoteTableModel.hpp"
#include "Spire/BookView/BookQuoteTableView.hpp"
#include "Spire/BookView/BookViewModel.hpp"
#include "Spire/Spire/Dimensions.hpp"

using namespace Nexus;
using namespace Spire;

BookViewTableWidget::BookViewTableWidget(const BookViewModel& model,
    BookViewProperties properties, QWidget* parent)
    : QWidget(parent) {
  m_layout = new QHBoxLayout(this);
  m_layout->setContentsMargins({});
  m_layout->setSpacing(scale_width(2));
  m_bid_table_view = new BookQuoteTableView(
    new BookQuoteTableModel(model, Side::BID, m_properties), this);
  m_layout->addWidget(m_bid_table_view);
  m_ask_table_view = new BookQuoteTableView(
    new BookQuoteTableModel(model, Side::ASK, m_properties), this);
  m_layout->addWidget(m_ask_table_view);
  set_properties(std::move(properties));
}

void BookViewTableWidget::set_properties(BookViewProperties properties) {
  m_properties = std::move(properties);
  m_bid_table_view->set_properties(m_properties);
  m_ask_table_view->set_properties(m_properties);
}
