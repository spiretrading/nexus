#include "spire/book_view/book_quote_table_model.hpp"
#include "Nexus/Definitions/BookQuote.hpp"
#include "spire/book_view/book_view_model.hpp"

using namespace Nexus;
using namespace Spire;

BookQuoteTableModel::BookQuoteTableModel(std::shared_ptr<BookViewModel> model,
    const Side& side, const BookViewProperties& properties)
    : m_model(std::move(model)),
      m_side(side),
      m_properties(properties) {
  if(m_side == Side::BID) {
    m_data = m_model->get_bids();
  } else {
    m_data = m_model->get_asks();
  }
}

int BookQuoteTableModel::rowCount(const QModelIndex& parent) const {
  return m_data.size();
}

int BookQuoteTableModel::columnCount(const QModelIndex& parent) const {
  return BookViewProperties::COLUMN_COUNT;
}

QVariant BookQuoteTableModel::data(const QModelIndex& index, int role) const {
  if(!index.isValid()) {
    return QVariant();
  }
  if(role == Qt::BackgroundRole) {
    return m_properties.get_book_quote_background_colors()[index.row()];
  } else if(role == Qt::ForegroundRole) {
    return m_properties.get_book_quote_foreground_color();
  } else if(role == Qt::FontRole) {
    return m_properties.get_book_quote_font();
  }
  return QVariant();
}

void BookQuoteTableModel::set_properties(
    const BookViewProperties& properties) {
  m_properties = properties;
}
