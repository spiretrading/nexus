#include "spire/book_view/book_quote_table_model.hpp"
#include "Nexus/Definitions/BookQuote.hpp"
#include "spire/book_view/book_view_model.hpp"
#include "spire/ui/custom_qt_variants.hpp"

using namespace Nexus;
using namespace Spire;
using Columns = BookViewProperties::Columns;

BookQuoteTableModel::BookQuoteTableModel(const BookViewModel& model,
    const Side& side, const BookViewProperties& properties)
    : m_model(&model),
      m_side(side),
      m_properties(properties) {
  auto& quotes = Pick(m_side, m_model->get_asks(), m_model->get_bids());
  for(auto& quote : quotes) {
    on_book_quote_signal(quote);
  }
  m_book_quote_connection = m_model->connect_book_quote_slot(
    [=] (const auto& b) { on_book_quote_signal(b); });
}

int BookQuoteTableModel::rowCount(const QModelIndex& parent) const {
  return static_cast<int>(m_data.size());
}

int BookQuoteTableModel::columnCount(const QModelIndex& parent) const {
  return BookViewProperties::COLUMN_COUNT;
}

QVariant BookQuoteTableModel::data(const QModelIndex& index, int role) const {
  if(!index.isValid()) {
    return QVariant();
  }
  if(role == Qt::DisplayRole) {
    switch(static_cast<Columns>(index.column())) {
      case Columns::MARKET_COLUMN:
        return QString::fromStdString(m_data[index.row()].m_mpid);
      case Columns::PRICE_COLUMN:
        return QVariant::fromValue(m_data[index.row()].m_quote.m_price);
      case Columns::SIZE_COLUMN:
        return QLocale().toString(
          static_cast<int>(m_data[index.row()].m_quote.m_size));
      default:
        return QVariant();
    }
  } else if(role == Qt::BackgroundRole) {
    auto market = m_data[index.row()].m_market;
    auto highlight = m_properties.get_market_highlight(market);
    auto& bg_colors = m_properties.get_book_quote_background_colors();
    if(highlight.is_initialized()) {
      if(highlight->m_highlight_all_levels) {
        return highlight->m_color;
      } else if(index.row() == (*m_market_first_index.find(market)).second) {
        return highlight->m_color;
      }
    }
    if(index.row() < bg_colors.size()) {
      return bg_colors[index.row()];
    } else {
      return bg_colors.back();
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
}

void BookQuoteTableModel::on_book_quote_signal(const BookQuote& book_quote) {
  if(book_quote.m_quote.m_side != m_side) {
    return;
  }
  auto it = std::lower_bound(m_data.begin(), m_data.end(), book_quote,
    BookQuoteListingComparator);
  if(it == m_data.end()) {
    if(book_quote.m_quote.m_size != 0) {
      beginInsertRows(QModelIndex(), m_data.size(), m_data.size());
      m_data.push_back(book_quote);
      endInsertRows();
    }
  } else if(std::tie(it->m_quote.m_price, it->m_mpid) ==
      std::tie(book_quote.m_quote.m_price, book_quote.m_mpid)) {
    auto index = std::distance(m_data.begin(), it);
    if(book_quote.m_quote.m_size == 0) {
      beginRemoveRows(QModelIndex(), index, index);
      m_data.erase(it);
      endRemoveRows();
    } else {
      *it = book_quote;
      dataChanged(createIndex(index, 0), createIndex(index,
        columnCount(QModelIndex())));
    }
  } else if(book_quote.m_quote.m_size != 0) {
    auto index = std::distance(m_data.begin(), it);
    beginInsertRows(QModelIndex(), index, index);
    m_data.insert(it, book_quote);
    endInsertRows();
  }
}
