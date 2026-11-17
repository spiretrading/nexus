#include "Spire/Blotter/BlotterTasksTableView.hpp"
#include <algorithm>
#include "Spire/Canvas/Common/CanvasNode.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/FieldValueModel.hpp"
#include "Spire/Spire/ListToTableModel.hpp"
#include "Spire/Spire/ListValueModel.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/EmptySelectionModel.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/TaskStateBox.hpp"
#include "Spire/Ui/ToggleButton.hpp"
#include "Spire/Ui/Ui.hpp"

using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  enum class TaskColumn {
    PINNED,
    NAME,
    ID,
    STATE
  };

  constexpr auto FIXED_COLUMNS = static_cast<int>(TaskColumn::STATE) + 1;

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
    auto monitor = index - FIXED_COLUMNS;
    if(monitor < std::ssize(entry.m_monitors) && entry.m_monitors[monitor]) {
      return entry.m_monitors[monitor]->get_value();
    }
    return {};
  }

  class TasksTableModel : public ListToTableModel<BlotterTaskEntry> {
    public:
      TasksTableModel(std::shared_ptr<TasksModel> tasks, int monitors)
          : ListToTableModel(
              std::move(tasks), FIXED_COLUMNS + monitors, &extract) {
        for(auto i = 0; i != get_row_size(); ++i) {
          add(i);
        }
        m_tasks_connection = get_source()->connect_operation_signal(
          std::bind_front(&TasksTableModel::on_task, this));
        m_table_connection = ListToTableModel::connect_operation_signal(
          [=, this] (const auto& operation) { m_signal(operation); });
      }

      connection connect_operation_signal(
          const OperationSignal::slot_type& slot) const override {
        return m_signal.connect(slot);
      }

    private:
      struct Row {
        int m_index;
        std::vector<scoped_connection> m_connections;
      };
      std::vector<std::unique_ptr<Row>> m_rows;
      mutable OperationSignal m_signal;
      scoped_connection m_tasks_connection;
      scoped_connection m_table_connection;

      void bind(Row& row) {
        row.m_connections.clear();
        auto& monitors = get_source()->get(row.m_index).m_monitors;
        for(auto i = 0; i < std::ssize(monitors) &&
            FIXED_COLUMNS + i < get_column_size(); ++i) {
          if(auto& observer = monitors[i]) {
            row.m_connections.emplace_back(observer->connect_update_signal(
              [=, this, &row, previous = observer->get_value()]
                  (const std::any& value) mutable {
                m_signal(UpdateOperation(row.m_index, FIXED_COLUMNS + i,
                  std::exchange(previous, value), value));
              }));
          }
        }
      }

      void reindex(int first) {
        for(auto i = first; i < std::ssize(m_rows); ++i) {
          m_rows[i]->m_index = i;
        }
      }

      void add(int index) {
        m_rows.insert(m_rows.begin() + index, std::make_unique<Row>());
        reindex(index);
        bind(*m_rows[index]);
      }

      void on_task(const TasksModel::Operation& operation) {
        visit(operation,
          [&] (const TasksModel::AddOperation& operation) {
            add(operation.m_index);
          },
          [&] (const TasksModel::MoveOperation& move) {
            move_element(m_rows, move.m_source, move.m_destination);
            reindex(std::min(move.m_source, move.m_destination));
          },
          [&] (const TasksModel::RemoveOperation& remove) {
            m_rows.erase(m_rows.begin() + remove.m_index);
            reindex(remove.m_index);
          },
          [&] (const TasksModel::UpdateOperation& update) {
            if(update.get_previous().m_monitors !=
                update.get_value().m_monitors) {
              bind(*m_rows[update.m_index]);
            }
          });
      }
  };
}

BlotterTasksTableView::BlotterTasksTableView(
    std::shared_ptr<TasksModel> tasks,
    std::shared_ptr<TaskMonitorsModel> monitors, QWidget* parent)
    : QWidget(parent),
      m_tasks(std::move(tasks)),
      m_monitors(std::move(monitors)) {
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_table_view = make_table();
  enclose(*this, *m_table_view);
  setFocusProxy(m_table_view);
  m_monitor_connection = m_monitors->connect_operation_signal(
    std::bind_front(&BlotterTasksTableView::on_monitor, this));
}

TableView* BlotterTasksTableView::make_table() const {
  auto table =
    std::make_shared<TasksTableModel>(m_tasks, m_monitors->get_size());
  auto pin_image =
    image_from_svg(":/Icons/blotter/tasks/pin-small.svg", scale(10, 10));
  auto builder = TableViewBuilder(table);
  builder.
    add_header_item(tr("Pinned")).
    add_header_item(tr("Name")).
    add_header_item(tr("ID")).
    add_header_item(tr("State"));
  for(auto& monitor : std::as_const(*m_monitors)) {
    builder.add_header_item(monitor.m_name);
  }
  auto table_view = builder.set_item_builder(
    [=, tasks = m_tasks] (const auto& table, auto row, auto index) ->
        QWidget* {
      auto column = static_cast<TaskColumn>(index);
      if(column == TaskColumn::PINNED) {
        auto entry = make_list_value_model(tasks, row);
        auto pin = make_icon_toggle_button(pin_image,
          make_field_value_model(entry, &BlotterTaskEntry::m_is_pinned));
        pin->setFixedSize(scale(26, 26));
        update_style(*pin, [] (auto& style) {
          style.get((Hover() || Press()) > Body()).
            set(BackgroundColor(Qt::transparent));
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
    set_comparator([] (const AnyRef& left, const AnyRef& right) {
      if(left.has_value() != right.has_value()) {
        return right.has_value();
      } else if(left.get_type() != right.get_type()) {
        return left.get_type().before(right.get_type());
      }
      return compare(left, right);
    }).
    set_selection(std::make_shared<TableSelectionModel>(
      std::make_shared<TableEmptySelectionModel>(),
      std::make_shared<ArrayListModel<int>>(),
      std::make_shared<ListEmptySelectionModel>())).
    make();
  table_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  update_style(table_view->get_body(), [] (auto& style) {
    style.get(Any() > CurrentColumn()).set(BackgroundColor(Qt::transparent));
    style.get(Any() > Row() > Current()).
      set(BackgroundColor(Qt::transparent)).
      set(border_color(QColor(Qt::transparent)));
  });
  auto pin_icon = new Icon(pin_image);
  pin_icon->setFixedSize(pin_image.size());
  auto pin_label = new QWidget();
  pin_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto pin_layout = make_hbox_layout(pin_label);
  pin_layout->setContentsMargins(0, scale_height(8), 0, scale_height(2));
  pin_layout->addWidget(pin_icon, 0, Qt::AlignVCenter);
  pin_layout->addStretch();
  link(*pin_label, *pin_icon);
  auto pinned_column = static_cast<int>(TaskColumn::PINNED);
  auto& header = table_view->get_header();
  auto pin_header = header.get_item(pinned_column);
  pin_header->set_is_resizeable(false);
  pin_header->setAccessibleName(tr("Pinned"));
  update_style(*pin_header, [] (auto& style) {
    style.get(Any() > TableHeaderItem::Separator()).set(Visibility::VISIBLE);
    style.get(Any() > TableHeaderItem::Label() > is_a<Icon>()).
      set(Fill(QColor(0xB8B8B8)));
    style.get(Hover() > TableHeaderItem::Label() > is_a<Icon>()).
      set(Fill(QColor(0x4B23A0)));
  });
  pin_header->set_label(*pin_label);
  header.get_widths()->set(
    pinned_column, std::max(scale_width(26), pin_header->sizeHint().width()));
  return table_view;
}

void BlotterTasksTableView::on_monitor(
    const TaskMonitorsModel::Operation& operation) {
  if(auto update =
      std::get_if<TaskMonitorsModel::UpdateOperation>(&operation)) {
    if(update->get_previous().m_name != update->get_value().m_name) {
      auto& items = *m_table_view->get_header().get_items();
      auto index = FIXED_COLUMNS + update->m_index;
      auto item = items.get(index);
      item.m_name = update->get_value().m_name;
      items.set(index, item);
    }
    return;
  }
  if(std::holds_alternative<TaskMonitorsModel::AddOperation>(operation) ||
      std::holds_alternative<TaskMonitorsModel::RemoveOperation>(operation) ||
      std::holds_alternative<TaskMonitorsModel::MoveOperation>(operation)) {
    delete m_table_view;
    m_table_view = make_table();
    layout()->addWidget(m_table_view);
    setFocusProxy(m_table_view);
  }
}
