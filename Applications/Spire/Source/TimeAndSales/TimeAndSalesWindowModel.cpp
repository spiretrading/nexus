#include "Spire/TimeAndSales/TimeAndSalesWindowModel.hpp"
#include <algorithm>
#include <QFontMetrics>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using Column = TimeAndSalesProperties::Column;
using PriceRange = TimeAndSalesProperties::PriceRange;

namespace {
  const auto LOADING_THRESHOLD = std::size_t{25};
  const auto SNAPSHOT_COUNT = 4 * LOADING_THRESHOLD;

  auto BREAKPOINT_PADDING() {
    static auto padding = scale_width(8);
    return padding;
  }

  auto get_breakpoint(const QFont& font, const QString& text) {
    auto width = QFontMetrics(font).horizontalAdvance(text);
    return 2 * BREAKPOINT_PADDING() + width;
  }
}

TimeAndSalesWindowModel::TimeAndSalesWindowModel(
    std::shared_ptr<TimeAndSalesModel> model,
    const TimeAndSalesProperties& properties)
    : m_model(std::move(model)),
      m_header_font("Roboto"),
      m_is_loading(false),
      m_is_fully_loaded(false) {
  set_properties(properties);
  m_header_font.setWeight(75);
  m_time_and_sale_connection = m_model.get()->connect_time_and_sale_signal(
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
  if(properties.m_font.pointSize() >= 11) {
    m_header_font.setPointSizeF(0.8 * properties.m_font.pointSize());
  } else {
    m_header_font.setPointSize(9);
  }
}

void TimeAndSalesWindowModel::set_column_size_reference(Column column,
    int size) {
  m_column_size_reference[column] = size;
  Q_EMIT headerDataChanged(Qt::Horizontal, static_cast<int>(column),
    static_cast<int>(column));
}

void TimeAndSalesWindowModel::set_row_visible(int row) {
  if(!m_is_fully_loaded && !m_is_loading &&
      m_entries.size() >= LOADING_THRESHOLD &&
      row >= static_cast<int>(m_entries.size() - LOADING_THRESHOLD)) {
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
  auto& entry = m_entries[(m_entries.size() - index.row()) - 1];
  if(role == Qt::DisplayRole) {
    auto& time_and_sale = entry.m_time_and_sale.GetValue();
    switch(static_cast<Column>(index.column())) {
      case Column::TIME_COLUMN:
        return QVariant::fromValue(time_and_sale.m_timestamp);
      case Column::PRICE_COLUMN:
        return QVariant::fromValue(time_and_sale.m_price);
      case Column::SIZE_COLUMN:
        return QVariant::fromValue(time_and_sale.m_size);
      case Column::MARKET_COLUMN:
        return QString::fromStdString(time_and_sale.m_marketCenter);
      case Column::CONDITION_COLUMN:
        return QString::fromStdString(time_and_sale.m_condition.m_code);
      default:
        return QVariant();
    }
  } else if(role == Qt::BackgroundRole) {
    return m_properties.get_band_color(entry.m_price_range);
  } else if(role == Qt::ForegroundRole) {
    return m_properties.get_text_color(entry.m_price_range);
  } else if(role == Qt::FontRole) {
    return m_properties.m_font;
  }
  return QVariant();
}

QVariant TimeAndSalesWindowModel::headerData(int section,
    Qt::Orientation orientation, int role) const {
  if(role == Qt::DisplayRole) {
    switch(static_cast<Column>(section)) {
      case Column::TIME_COLUMN:
        return tr("Time");
      case Column::PRICE_COLUMN:
        return tr("Price");
      case Column::SIZE_COLUMN:
        if(is_short_text(Column::SIZE_COLUMN,
            get_breakpoint(m_header_font, tr("Quantity")))) {
          return tr("Qty");
        }
        return tr("Quantity");
      case Column::MARKET_COLUMN:
        if(is_short_text(Column::MARKET_COLUMN,
            get_breakpoint(m_header_font, tr("Market")))) {
          return tr("Mkt");
        }
        return tr("Market");
      case Column::CONDITION_COLUMN:
        if(is_short_text(Column::CONDITION_COLUMN,
            get_breakpoint(m_header_font, tr("Condition")))) {
          return tr("Cond");
        }
        return tr("Condition");
      default:
        return QVariant();
    }
  } else if(role == Qt::FontRole) {
    return m_header_font;
  }
  return QVariant();
}

bool TimeAndSalesWindowModel::is_short_text(Column column,
    int breakpoint) const {
  auto iter = m_column_size_reference.find(column);
  if(iter != m_column_size_reference.end()) {
    return iter->second < breakpoint;
  }
  return false;
}

void TimeAndSalesWindowModel::update_data(const TimeAndSalesModel::Entry& e) {
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
