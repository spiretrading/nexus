#include "Spire/Blotter/MonitoredTasksModel.hpp"
#include "Spire/Canvas/SystemNodes/CanvasObserver.hpp"

using namespace Spire;

namespace {
  auto make_observer(
      const std::shared_ptr<Task>& task, const BlotterTaskMonitor& monitor) {
    if(monitor.m_expression) {
      return std::make_shared<CanvasObserver>(task, *monitor.m_expression);
    }
    return std::shared_ptr<CanvasObserver>();
  }
}

MonitoredTasksModel::MonitoredTasksModel(
    std::shared_ptr<TaskMonitorsModel> monitors)
    : m_monitors(std::move(monitors)) {
  m_connection = m_monitors->connect_operation_signal(
    std::bind_front(&MonitoredTasksModel::on_monitor, this));
}

MonitoredTasksModel::MonitoredTasksModel(std::shared_ptr<TasksModel> source,
    std::shared_ptr<TaskMonitorsModel> monitors)
    : MonitoredTasksModel(std::move(monitors)) {
  m_source = std::move(source);
  for(auto& value : std::as_const(*m_source)) {
    auto entry = value;
    entry.m_is_pinned = false;
    ArrayListModel::insert(observe(entry), get_size());
  }
  m_source_connection = m_source->connect_operation_signal(
    std::bind_front(&MonitoredTasksModel::on_source, this));
}

QValidator::State MonitoredTasksModel::set(int index, const Type& value) {
  auto& current = get(index);
  if(m_source) {
    if(value.m_task != current.m_task || value.m_state != current.m_state) {
      return QValidator::Invalid;
    }
    auto entry = current;
    entry.m_is_pinned = value.m_is_pinned;
    return ArrayListModel::set(index, entry);
  }
  if(value.m_task != current.m_task) {
    return ArrayListModel::set(index, observe(value));
  }
  auto entry = value;
  entry.m_monitors = current.m_monitors;
  return ArrayListModel::set(index, entry);
}

QValidator::State MonitoredTasksModel::insert(const Type& value, int index) {
  if(m_source) {
    return QValidator::Invalid;
  }
  return ArrayListModel::insert(observe(value), index);
}

QValidator::State MonitoredTasksModel::move(int source, int destination) {
  if(m_source) {
    return QValidator::Invalid;
  }
  return ArrayListModel::move(source, destination);
}

QValidator::State MonitoredTasksModel::remove(int index) {
  if(m_source) {
    return QValidator::Invalid;
  }
  return ArrayListModel::remove(index);
}

BlotterTaskEntry MonitoredTasksModel::observe(BlotterTaskEntry entry) const {
  entry.m_monitors.clear();
  for(auto& monitor : std::as_const(*m_monitors)) {
    entry.m_monitors.push_back(make_observer(entry.m_task, monitor));
  }
  return entry;
}

void MonitoredTasksModel::on_monitor(
    const TaskMonitorsModel::Operation& operation) {
  auto update = [&] (auto change) {
    transact([&] {
      for(auto i = 0; i != get_size(); ++i) {
        auto entry = get(i);
        change(entry);
        ArrayListModel::set(i, entry);
      }
    });
  };
  visit(operation,
    [&] (const TaskMonitorsModel::AddOperation& add) {
      update([&] (auto& entry) {
        entry.m_monitors.insert(entry.m_monitors.begin() + add.m_index,
          make_observer(entry.m_task, m_monitors->get(add.m_index)));
      });
    },
    [&] (const TaskMonitorsModel::RemoveOperation& remove) {
      update([&] (auto& entry) {
        entry.m_monitors.erase(entry.m_monitors.begin() + remove.m_index);
      });
    },
    [&] (const TaskMonitorsModel::MoveOperation& move) {
      update([&] (auto& entry) {
        move_element(entry.m_monitors, move.m_source, move.m_destination);
      });
    },
    [&] (const TaskMonitorsModel::UpdateOperation& change) {
      if(change.get_previous().m_expression !=
          change.get_value().m_expression) {
        update([&] (auto& entry) {
          entry.m_monitors[change.m_index] =
            make_observer(entry.m_task, change.get_value());
        });
      }
    });
}

void MonitoredTasksModel::on_source(const TasksModel::Operation& operation) {
  visit(operation,
    [&] (StartTransaction) {
      start_transaction();
    },
    [&] (EndTransaction) {
      end_transaction();
    },
    [&] (const AddOperation& add) {
      auto entry = m_source->get(add.m_index);
      entry.m_is_pinned = false;
      ArrayListModel::insert(observe(entry), add.m_index);
    },
    [&] (const MoveOperation& move) {
      ArrayListModel::move(move.m_source, move.m_destination);
    },
    [&] (const RemoveOperation& remove) {
      ArrayListModel::remove(remove.m_index);
    },
    [&] (const UpdateOperation& update) {
      auto& current = get(update.m_index);
      auto entry = update.get_value();
      if(entry.m_task == current.m_task) {
        entry.m_is_pinned = current.m_is_pinned;
        entry.m_monitors = current.m_monitors;
      } else {
        entry.m_is_pinned = false;
        entry = observe(entry);
      }
      ArrayListModel::set(update.m_index, entry);
    });
}
