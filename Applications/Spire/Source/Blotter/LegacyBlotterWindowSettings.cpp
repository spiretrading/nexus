#include "Spire/Blotter/LegacyBlotterWindowSettings.hpp"
#include <algorithm>
#include <array>
#include <QHeaderView>
#include <QStandardItemModel>
#include "Spire/Blotter/BlotterWindowSettings.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "Spire/Ui/TableView.hpp"

using namespace Beam;
using namespace boost;
using namespace Spire;

namespace {
  optional<double> get_split_ratio(const QByteArray& state) {
    auto stream = QDataStream(state);
    auto marker = qint32(0);
    auto version = qint32(0);
    auto count = quint32(0);
    auto primary = qint32(0);
    auto secondary = qint32(0);
    stream >> marker >> version >> count >> primary >> secondary;
    constexpr auto SPLITTER_MAGIC = 0xff;
    if(stream.status() != QDataStream::Ok || marker != SPLITTER_MAGIC ||
        version < 0 || version > 1 || count != 2 || primary < 0 ||
        secondary < 0 || (primary == 0 && secondary == 0)) {
      return none;
    }
    return primary / (static_cast<double>(primary) + secondary);
  }
}

std::string LegacyBlotterWindowSettings::GetName() const {
  return "Blotter - " + m_blotter_name;
}

QWidget* LegacyBlotterWindowSettings::Reopen(
    Ref<UserProfile> user_profile) const {
  auto& registry = user_profile->GetBlotterRegistry();
  for(auto& model : std::as_const(*registry.get_blotters())) {
    if(model->get_name()->get().toStdString() == m_blotter_name) {
      auto& window = registry.get(*model).open();
      Apply(user_profile, out(window));
      return &window;
    }
  }
  return nullptr;
}

void LegacyBlotterWindowSettings::Apply(
    Ref<UserProfile> user_profile, Out<QWidget> widget) const {
  auto& window = dynamic_cast<BlotterWindow&>(*widget);
  auto settings = BlotterWindowSettings(window);
  settings.m_geometry = m_geometry;
  if(auto ratio = get_split_ratio(m_splitter_state)) {
    settings.m_split = *ratio;
  }
  auto convert = [&] (const TableState& legacy, const auto& columns,
      BlotterWindowSettings::TableState& table) {
    auto model = QStandardItemModel(0, static_cast<int>(columns.size()));
    auto header = QHeaderView(Qt::Horizontal);
    header.setModel(&model);
    header.restoreGeometry(legacy.m_table_geometry);
    if(!header.restoreState(legacy.m_table_state) ||
        header.count() != static_cast<int>(columns.size())) {
      return;
    }
    auto converted = std::vector<BlotterWindowSettings::Column>();
    table.m_sort.clear();
    for(auto visual = 0; visual != header.count(); ++visual) {
      auto index = header.logicalIndex(visual);
      auto column = BlotterWindowSettings::Column();
      column.m_index = columns[index];
      column.m_is_visible = !header.isSectionHidden(index);
      header.setSectionHidden(index, false);
      column.m_width = unscale_width(header.sectionSize(index));
      column.m_order = static_cast<int>(TableHeaderItem::Order::NONE);
      if(header.isSortIndicatorShown() &&
          index == header.sortIndicatorSection()) {
        auto sort = BlotterWindowSettings::Sort();
        sort.m_index = column.m_index;
        if(header.sortIndicatorOrder() == Qt::AscendingOrder) {
          column.m_order =
            static_cast<int>(TableHeaderItem::Order::ASCENDING);
          sort.m_order =
            static_cast<int>(SortedTableModel::Ordering::ASCENDING);
        } else {
          column.m_order =
            static_cast<int>(TableHeaderItem::Order::DESCENDING);
          sort.m_order =
            static_cast<int>(SortedTableModel::Ordering::DESCENDING);
        }
        table.m_sort.push_back(sort);
      }
      converted.push_back(column);
    }
    for(auto column : table.m_columns) {
      if(std::ranges::find(columns, column.m_index) == columns.end()) {
        column.m_order = static_cast<int>(TableHeaderItem::Order::NONE);
        converted.push_back(column);
      }
    }
    table.m_columns = std::move(converted);
  };
  convert(m_open_positions_widget_state, std::array{0, 1, 2, 3, 4, 5, 6},
    settings.m_positions);
  convert(m_order_log_widget_state,
    std::array{0, 1, 2, 3, 9, 7, 4, 8, 6, 5, 10}, settings.m_orders);
  convert(m_activity_log_widget_state,
    std::array{0, 1, 2, 3, 4, 6, 8, 9, 10, 11, 12, 13, 14},
    settings.m_executions);
  settings.Apply(user_profile, out(window));
  if(!m_geometry.isEmpty()) {
    auto frame_height = GetSystemMetrics(SM_CYCAPTION) +
      GetSystemMetrics(SM_CYFRAME) + GetSystemMetrics(SM_CXPADDEDBORDER);
    window.move(window.x(), window.y() - frame_height);
    window.resize(window.width(), window.height() + frame_height);
  }
}
