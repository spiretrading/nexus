#ifndef SPIRE_MONITORED_TASKS_MODEL_HPP
#define SPIRE_MONITORED_TASKS_MODEL_HPP
#include "Spire/Blotter/BlotterTaskEntry.hpp"
#include "Spire/Blotter/BlotterTaskMonitor.hpp"
#include "Spire/Spire/ArrayListModel.hpp"

namespace Spire {

  /** Maintains the observers for a blotter's task entries. */
  class MonitoredTasksModel : public ArrayListModel<BlotterTaskEntry> {
    public:

      /**
       * Constructs an empty task list.
       * @param monitors The monitor definitions to apply to every task.
       */
      explicit MonitoredTasksModel(std::shared_ptr<TaskMonitorsModel> monitors);

      QValidator::State set(int index, const Type& value) override;
      QValidator::State insert(const Type& value, int index) override;
      using ArrayListModel::insert;

    private:
      std::shared_ptr<TaskMonitorsModel> m_monitors;
      boost::signals2::scoped_connection m_connection;

      BlotterTaskEntry observe(BlotterTaskEntry entry) const;
      void on_monitor(const TaskMonitorsModel::Operation& operation);
  };
}

#endif
