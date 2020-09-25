#include "Spire/BookView/BookQuoteTableModel.hpp"
#include "Nexus/Definitions/BookQuote.hpp"
#include "Spire/BookView/BookViewModel.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

using namespace Nexus;
using namespace Spire;
using Column = BookViewProperties::Column;

BookQuoteTableModel::BookQuoteTableModel(const BookViewModel& model,
    Side side, const BookViewProperties& properties)
    : m_model(&model),
      m_side(side),
      m_properties(properties),
      m_size(0) {
  for(auto& entry : m_model->get_market_database().GetEntries()) {
    m_first_market_index[entry.m_code] = std::numeric_limits<int>::max();
  }
  auto& quotes = Pick(m_side, m_model->get_asks(), m_model->get_bids());
  for(auto i = quotes.rbegin(); i != quotes.rend(); ++i) {
    on_quote_signal(**i, std::distance(quotes.rbegin(), i));
  }
  m_quote_connection = m_model->connect_quote_slot(
    [=] (const auto& quote, auto index) { on_quote_signal(quote, index); });
}

int BookQuoteTableModel::rowCount(const QModelIndex& parent) const {
  return m_size;
}

int BookQuoteTableModel::columnCount(const QModelIndex& parent) const {
  return BookViewProperties::COLUMN_COUNT;
}

QVariant BookQuoteTableModel::data(const QModelIndex& index, int role) const {
  if(!index.isValid()) {
    return QVariant();
  }
  auto& book = Pick(m_side, m_model->get_asks(), m_model->get_bids());
  auto& quote = *book[m_size - 1 - index.row()];
  if(role == Qt::DisplayRole) {
    switch(static_cast<Column>(index.column())) {
      case Column::MARKET_COLUMN:
        return QString::fromStdString(quote.m_quote.m_mpid);
      case Column::PRICE_COLUMN:
        return QVariant::fromValue(quote.m_quote.m_quote.m_price);
      case Column::SIZE_COLUMN:
        return QVariant::fromValue(quote.m_quote.m_quote.m_size);
      default:
        return QVariant();
    }
  } else if(role == Qt::BackgroundRole) {
    auto& market = quote.m_quote.m_market;
    auto highlight = m_properties.get_market_highlight(market);
    auto& background_colors = m_properties.get_book_quote_background_colors();
    if(highlight.is_initialized()) {
      if(highlight->m_highlight_all_levels) {
        return highlight->m_color;
      } else if(index.row() == m_first_market_index.find(market)->second) {
        return highlight->m_color;
      }
    }
    if(quote.m_price_level < static_cast<int>(background_colors.size())) {
      return background_colors[quote.m_price_level];
    } else {
      return background_colors.back();
    }
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
  update_first_market_indexes(0);
}

void BookQuoteTableModel::on_quote_signal(const BookViewModel::Quote& quote,
    int index) {
  if(quote.m_quote.m_quote.m_side != m_side) {
    return;
  }
  auto& book = Pick(m_side, m_model->get_asks(), m_model->get_bids());
  auto test_price_levels = false;
  if(m_size > static_cast<int>(book.size())) {
    beginRemoveRows(QModelIndex(), index, index);
    --m_size;
    test_price_levels = true;
    endRemoveRows();
  } else if(m_size == book.size()) {
    dataChanged(createIndex(index, 0), createIndex(index,
      columnCount(QModelIndex())));
  } else {
    beginInsertRows(QModelIndex(), index, index);
    ++m_size;
    test_price_levels = true;
    endInsertRows();
  }
  if(test_price_levels && index != m_size &&
      quote.m_quote.m_quote.m_price != book[index]->m_quote.m_quote.m_price &&
      (index == 0 || quote.m_quote.m_quote.m_price !=
      book[index - 1]->m_quote.m_quote.m_price)) {
    dataChanged(createIndex(index, 0), createIndex(m_size - 1,
      columnCount(QModelIndex())));
  }
  update_first_market_indexes(index);
}

void BookQuoteTableModel::update_first_market_indexes(int index) {
  auto& book = Pick(m_side, m_model->get_asks(), m_model->get_bids());
  for(auto& entry : m_model->get_market_database().GetEntries()) {
    if(m_first_market_index[entry.m_code] >=
        std::distance(book.rbegin(), book.rbegin() + index)) {
      m_first_market_index[entry.m_code] = INT_MAX;
    }
  }
  for(auto i = book.rbegin() + index; i != book.rend(); ++i) {
    auto dist = std::distance(book.rbegin(), i);
    auto& index = m_first_market_index[(**i).m_quote.m_market];
    if(dist < index) {
      index = dist;
    }
  }
}
