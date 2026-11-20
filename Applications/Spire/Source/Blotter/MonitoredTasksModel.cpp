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

QValidator::State MonitoredTasksModel::set(int index, const Type& value) {
  auto& current = get(index);
  if(value.m_task != current.m_task) {
    return ArrayListModel::set(index, observe(value));
  }
  auto entry = value;
  entry.m_monitors = current.m_monitors;
  return ArrayListModel::set(index, entry);
}

QValidator::State MonitoredTasksModel::insert(const Type& value, int index) {
  return ArrayListModel::insert(observe(value), index);
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
