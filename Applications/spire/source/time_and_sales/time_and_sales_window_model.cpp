#include "spire/time_and_sales/time_and_sales_window_model.hpp"
#include <algorithm>
#include "spire/ui/custom_qt_variants.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace spire;
using columns = time_and_sales_properties::columns;
using price_range = time_and_sales_properties::price_range;

namespace {
  const auto LOADING_THRESHOLD = 25;
  const auto SNAPSHOT_COUNT = 4 * LOADING_THRESHOLD;
}

time_and_sales_window_model::time_and_sales_window_model(
    std::shared_ptr<time_and_sales_model> model,
    const time_and_sales_properties& properties)
    : m_model(std::move(model)),
      m_is_loading(false),
      m_is_fully_loaded(false) {
  set_properties(properties);
  m_model.get()->connect_time_and_sale_signal(
    [=] (auto e) { update_data(e); });
  load_snapshot(Beam::Queries::Sequence::Last());
}

const Security& time_and_sales_window_model::get_security() const {
  return m_model->get_security();
}

bool time_and_sales_window_model::is_loading() const {
  return m_is_loading;
}

void time_and_sales_window_model::set_properties(
    const time_and_sales_properties& properties) {
  m_properties = properties;
}

void time_and_sales_window_model::set_row_visible(int row) {
  if(!m_is_fully_loaded && !m_is_loading &&
      m_entries.size() >= LOADING_THRESHOLD &&
      row >= m_entries.size() - LOADING_THRESHOLD) {
    load_snapshot(m_entries.front().m_time_and_sale.GetSequence());
  }
}

connection time_and_sales_window_model::connect_begin_loading_signal(
    const begin_loading_signal::slot_type& slot) const {
  return m_begin_loading_signal.connect(slot);
}

connection time_and_sales_window_model::connect_end_loading_signal(
    const end_loading_signal::slot_type& slot) const {
  return m_end_loading_signal.connect(slot);
}

int time_and_sales_window_model::rowCount(const QModelIndex& parent) const {
  return m_entries.size();
}

int time_and_sales_window_model::columnCount(const QModelIndex& parent) const {
  return time_and_sales_properties::COLUMN_COUNT;
}

QVariant time_and_sales_window_model::data(const QModelIndex& index,
    int role) const {
  if(!index.isValid()) {
    return QVariant();
  }
  auto row_index = (m_entries.size() - index.row()) - 1;
  if(role == Qt::DisplayRole) {
    switch(static_cast<columns>(index.column())) {
      case columns::TIME_COLUMN:
        return QVariant::fromValue(
          m_entries[row_index].m_time_and_sale.GetValue().m_timestamp);
      case columns::PRICE_COLUMN:
        return QVariant::fromValue(
          m_entries[row_index].m_time_and_sale.GetValue().m_price);
      case columns::SIZE_COLUMN:
        return QVariant::fromValue(
          m_entries[row_index].m_time_and_sale.GetValue().m_size);
      case columns::MARKET_COLUMN:
        return QString::fromStdString(
          m_entries[row_index].m_time_and_sale.GetValue().m_marketCenter);
      case columns::CONDITION_COLUMN:
        return QString::fromStdString(
          m_entries[row_index].m_time_and_sale.GetValue().m_condition.m_code);
      default:
        return QVariant();
    }
  } else if(role == Qt::BackgroundRole) {
    return m_properties.get_band_color(m_entries[row_index].m_price_range);
  } else if(role == Qt::ForegroundRole) {
    return m_properties.get_text_color(m_entries[row_index].m_price_range);
  } else if(role == Qt::FontRole) {
    return m_properties.m_font;
  }
  return QVariant();
}

QVariant time_and_sales_window_model::headerData(int section,
    Qt::Orientation orientation, int role) const {
  if(role == Qt::DisplayRole) {
    switch(static_cast<columns>(section)) {
      case columns::TIME_COLUMN:
        return tr("Time");
      case columns::PRICE_COLUMN:
        return tr("Price");
      case columns::SIZE_COLUMN:
        return tr("Qty");
      case columns::MARKET_COLUMN:
        return tr("Mkt");
      case columns::CONDITION_COLUMN:
        return tr("Cond");
      default:
        return QVariant();
    }
  }
  return QVariant();
}

void time_and_sales_window_model::update_data(
    const time_and_sales_model::entry& e) {
  beginInsertRows(QModelIndex(), 0, 0);
  m_entries.push_back(e);
  endInsertRows();
}

void time_and_sales_window_model::load_snapshot(Beam::Queries::Sequence last) {
  m_is_loading = true;
  m_begin_loading_signal();
  m_snapshot_promise = m_model->load_snapshot(last, SNAPSHOT_COUNT);
  m_snapshot_promise.then(
    [=] (std::vector<time_and_sales_model::entry> snapshot) {
      if(snapshot.empty()) {
        m_is_fully_loaded = true;
      } else {
        beginInsertRows(QModelIndex(), m_entries.size(),
          m_entries.size() + snapshot.size() - 1);
        m_entries.insert(m_entries.begin(),
          std::make_move_iterator(snapshot.begin()),
          std::make_move_iterator(snapshot.end()));
        endInsertRows();
      }
      m_is_loading = false;
      m_end_loading_signal();
    });
}
