#include "Spire/Blotter/BlotterWindowSettings.hpp"
#include <algorithm>
#include <cmath>
#include "Spire/Blotter/BlotterExecutionsView.hpp"
#include "Spire/Blotter/BlotterProfitLossView.hpp"
#include "Spire/Blotter/BlotterTabView.hpp"
#include "Spire/Blotter/BlotterTasksTableView.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "Spire/Ui/SplitView.hpp"
#include "Spire/Ui/TabView.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Spire;
using namespace Spire::LegacyUI;

BlotterWindowSettings::TableState::TableState(TableView& table) {
  auto& header = table.get_header();
  for(auto i = 0; i != header.get_items()->get_size(); ++i) {
    m_columns.push_back(Column(header.get_column_order()->get(i),
      unscale_width(header.get_widths()->get(i)),
      !header.get_item(i)->isHidden(),
      static_cast<int>(header.get_items()->get(i).m_order)));
  }
  for(auto& order : table.get_column_order()) {
    m_sort.push_back(Sort(order.m_index, static_cast<int>(order.m_order)));
  }
}

void BlotterWindowSettings::TableState::apply(TableView& table) const {
  auto& header = table.get_header();
  auto& items = *header.get_items();
  auto destination = 0;
  auto seen = std::vector<bool>(items.get_size());
  for(auto& column : m_columns) {
    if(column.m_index < 0 || column.m_index >= items.get_size() ||
        seen[column.m_index]) {
      continue;
    }
    seen[column.m_index] = true;
    auto order = header.get_column_order();
    auto i = std::ranges::find(std::as_const(*order), column.m_index);
    items.move(static_cast<int>(i - order->cbegin()), destination);
    if(column.m_width > 0) {
      header.get_widths()->set(destination, scale_width(column.m_width));
    }
    if(column.m_is_visible) {
      table.show_column(destination);
    } else {
      table.hide_column(destination);
    }
    auto item = items.get(destination);
    if(column.m_order >= static_cast<int>(TableHeaderItem::Order::NONE) &&
        column.m_order <=
          static_cast<int>(TableHeaderItem::Order::DESCENDING)) {
      item.m_order = static_cast<TableHeaderItem::Order>(column.m_order);
      items.set(destination, item);
    }
    ++destination;
  }
  auto sort = std::vector<SortedTableModel::ColumnOrder>();
  for(auto& order : m_sort) {
    if(order.m_index >= 0 && order.m_index < items.get_size() &&
        order.m_order >= static_cast<int>(SortedTableModel::Ordering::NONE) &&
        order.m_order <=
          static_cast<int>(SortedTableModel::Ordering::DESCENDING)) {
      sort.push_back(SortedTableModel::ColumnOrder(order.m_index,
        static_cast<SortedTableModel::Ordering>(order.m_order)));
    }
  }
  table.set_column_order(std::move(sort));
}

BlotterWindowSettings::BlotterWindowSettings()
  : m_split(0.5),
    m_tab(0) {}

BlotterWindowSettings::BlotterWindowSettings(const BlotterWindow& window)
  : m_id(window.m_id),
    m_name(window.get_model()->get_name()->get().toStdString()),
    m_geometry(window.saveGeometry()),
    m_split(window.m_split_view->get_primary_ratio()),
    m_tab(window.m_tab_view->m_tab_view->get_current()->get().value_or(0)),
    m_tasks(*window.m_tasks_view->m_table->m_table_view),
    m_positions(*window.m_tab_view->m_positions->m_table),
    m_profit_loss(*window.m_tab_view->m_profit_loss->m_table),
    m_orders(*window.m_tab_view->m_order_log->m_table),
    m_executions(*window.m_tab_view->m_executions->m_table),
    m_statuses(window.m_tab_view->m_executions->m_statuses->cbegin(),
      window.m_tab_view->m_executions->m_statuses->cend()) {}

std::string BlotterWindowSettings::GetName() const {
  return "Blotter - " + m_name;
}

QWidget* BlotterWindowSettings::Reopen(Ref<UserProfile> profile) const {
  auto& registry = profile->GetBlotterRegistry();
  for(auto& model : std::as_const(*registry.get_blotters())) {
    auto& controller = registry.get(*model);
    if(controller.m_id == m_id) {
      auto settings = *this;
      auto& window = controller.open();
      settings.Apply(profile, out(window));
      return &window;
    }
  }
  return nullptr;
}

void BlotterWindowSettings::Apply(
    Ref<UserProfile> profile, Out<QWidget> widget) const {
  auto& window = dynamic_cast<BlotterWindow&>(*widget);
  restore_geometry(window, m_geometry);
  if(std::isfinite(m_split) && m_split >= 0 && m_split <= 1) {
    window.m_split_view->set_primary_ratio(m_split);
  }
  m_tasks.apply(*window.m_tasks_view->m_table->m_table_view);
  m_positions.apply(*window.m_tab_view->m_positions->m_table);
  m_profit_loss.apply(*window.m_tab_view->m_profit_loss->m_table);
  m_orders.apply(*window.m_tab_view->m_order_log->m_table);
  m_executions.apply(*window.m_tab_view->m_executions->m_table);
  auto& statuses = *window.m_tab_view->m_executions->m_statuses;
  statuses.transact([&] {
    while(statuses.get_size() != 0) {
      statuses.remove(statuses.get_size() - 1);
    }
    for(auto status : m_statuses) {
      statuses.push(status);
    }
  });
  if(m_tab >= 0 && m_tab < 5) {
    window.m_tab_view->m_tab_view->get_current()->set(m_tab);
  }
}
