#include "spire/time_and_sales/time_and_sales_window_model.hpp"
#include <algorithm>
#include "spire/ui/custom_qt_variants.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using Columns = TimeAndSalesProperties::Columns;
using PriceRange = TimeAndSalesProperties::PriceRange;

namespace {
  const auto LOADING_THRESHOLD = 25;
  const auto SNAPSHOT_COUNT = 4 * LOADING_THRESHOLD;
}

TimeAndSalesWindowModel::TimeAndSalesWindowModel(
    std::shared_ptr<TimeAndSalesModel> model,
    const TimeAndSalesProperties& properties)
    : m_model(std::move(model)),
      m_is_loading(false),
      m_is_fully_loaded(false) {
  set_properties(properties);
  m_model.get()->connect_time_and_sale_signal(
    [=] (auto e) { update_data(e); });
  load_snapshot(Beam::Queries::Sequence::Present());
}

const Security& TimeAndSalesWindowModel::get_security() const {
  return m_model->get_security();
}

bool TimeAndSalesWindowModel::is_loading() const {
  return m_is_loading;
}

void TimeAndSalesWindowModel::set_properties(
    const TimeAndSalesProperties& properties) {
  m_properties = properties;
}

void TimeAndSalesWindowModel::set_row_visible(int row) {
  if(!m_is_fully_loaded && !m_is_loading &&
      m_entries.size() >= LOADING_THRESHOLD &&
      row >= m_entries.size() - LOADING_THRESHOLD) {
    load_snapshot(m_entries.front().m_time_and_sale.GetSequence());
  }
}

connection TimeAndSalesWindowModel::connect_begin_loading_signal(
    const BeginLoadingSignal::slot_type& slot) const {
  return m_begin_loading_signal.connect(slot);
}

connection TimeAndSalesWindowModel::connect_end_loading_signal(
    const EndLoadingSignal::slot_type& slot) const {
  return m_end_loading_signal.connect(slot);
}

int TimeAndSalesWindowModel::rowCount(const QModelIndex& parent) const {
  return m_entries.size();
}

int TimeAndSalesWindowModel::columnCount(const QModelIndex& parent) const {
  return TimeAndSalesProperties::COLUMN_COUNT;
}

QVariant TimeAndSalesWindowModel::data(const QModelIndex& index,
    int role) const {
  if(!index.isValid()) {
    return QVariant();
  }
  auto row_index = (m_entries.size() - index.row()) - 1;
  if(role == Qt::DisplayRole) {
    switch(static_cast<Columns>(index.column())) {
      case Columns::TIME_COLUMN:
        return QVariant::fromValue(
          m_entries[row_index].m_time_and_sale.GetValue().m_timestamp);
      case Columns::PRICE_COLUMN:
        return QVariant::fromValue(
          m_entries[row_index].m_time_and_sale.GetValue().m_price);
      case Columns::SIZE_COLUMN:
        return QVariant::fromValue(
          m_entries[row_index].m_time_and_sale.GetValue().m_size);
      case Columns::MARKET_COLUMN:
        return QString::fromStdString(
          m_entries[row_index].m_time_and_sale.GetValue().m_marketCenter);
      case Columns::CONDITION_COLUMN:
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

QVariant TimeAndSalesWindowModel::headerData(int section,
    Qt::Orientation orientation, int role) const {
  if(role == Qt::DisplayRole) {
    switch(static_cast<Columns>(section)) {
      case Columns::TIME_COLUMN:
        return tr("Time");
      case Columns::PRICE_COLUMN:
        return tr("Price");
      case Columns::SIZE_COLUMN:
        return tr("Qty");
      case Columns::MARKET_COLUMN:
        return tr("Mkt");
      case Columns::CONDITION_COLUMN:
        return tr("Cond");
      default:
        return QVariant();
    }
  }
  return QVariant();
}

void TimeAndSalesWindowModel::update_data(
    const TimeAndSalesModel::Entry& e) {
  beginInsertRows(QModelIndex(), 0, 0);
  m_entries.push_back(e);
  endInsertRows();
}

void TimeAndSalesWindowModel::load_snapshot(Beam::Queries::Sequence last) {
  m_is_loading = true;
  m_begin_loading_signal();
  m_snapshot_promise = m_model->load_snapshot(last, SNAPSHOT_COUNT);
  m_snapshot_promise.then(
    [=] (std::vector<TimeAndSalesModel::Entry> snapshot) {
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
