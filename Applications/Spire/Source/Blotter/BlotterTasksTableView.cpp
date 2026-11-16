#include "Spire/Blotter/BlotterTasksTableView.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/FieldValueModel.hpp"
#include "Spire/Spire/ListToTableModel.hpp"
#include "Spire/Spire/ListValueModel.hpp"
#include "Spire/Ui/EmptySelectionModel.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/TableView.hpp"
#include "Spire/Ui/TaskStateBox.hpp"
#include "Spire/Ui/ToggleButton.hpp"
#include "Spire/Ui/Ui.hpp"

using namespace Spire;
using namespace Spire::Styles;

namespace {
  enum class TaskColumn {
    PINNED,
    NAME,
    ID,
    STATE
  };

  AnyRef extract(BlotterTaskEntry& entry, int index) {
    auto column = static_cast<TaskColumn>(index);
    if(column == TaskColumn::PINNED) {
      return entry.m_is_pinned;
    } else if(column == TaskColumn::NAME) {
      return QString::fromStdString(entry.m_task->GetNode().GetText());
    } else if(column == TaskColumn::ID) {
      return entry.m_task->GetId();
    } else if(column == TaskColumn::STATE) {
      return std::as_const(entry.m_state);
    }
    return QString();
  }
}

BlotterTasksTableView::BlotterTasksTableView(
    std::shared_ptr<TasksModel> tasks, QWidget* parent)
    : QWidget(parent) {
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto column_count = static_cast<int>(TaskColumn::STATE) + 1;
  auto table = std::make_shared<ListToTableModel<BlotterTaskEntry>>(
    tasks, column_count, &extract);
  auto table_view = TableViewBuilder(table).
    add_header_item(tr("Pinned")).
    add_header_item(tr("Name")).
    add_header_item(tr("ID")).
    add_header_item(tr("State")).
    set_item_builder([=] (const auto& table, auto row, auto index) -> QWidget* {
      auto column = static_cast<TaskColumn>(index);
      if(column == TaskColumn::PINNED) {
        auto entry = make_list_value_model(tasks, row);
        auto pin = make_icon_toggle_button(
          image_from_svg(":/Icons/blotter/tasks/pin.svg", scale(26, 26)),
          make_field_value_model(entry, &BlotterTaskEntry::m_is_pinned),
          tr("Pin task"));
        pin->setFixedSize(scale(26, 26));
        update_style(*pin, [] (auto& style) {
          style.get(Any() > is_a<Icon>()).set(Fill(QColor(0xB8B8B8)));
        });
        return pin;
      } else if(column == TaskColumn::STATE) {
        auto entry = make_list_value_model(tasks, row);
        auto state = make_task_state_box(
          make_field_value_model(entry, &BlotterTaskEntry::m_state));
        state->set_read_only(true);
        return state;
      }
      return TableView::default_item_builder(table, row, index);
    }).
    set_selection(std::make_shared<TableSelectionModel>(
      std::make_shared<TableEmptySelectionModel>(),
      std::make_shared<ArrayListModel<int>>(),
      std::make_shared<ListEmptySelectionModel>())).
    make();
  table_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  enclose(*this, *table_view);
  setFocusProxy(table_view);
}
