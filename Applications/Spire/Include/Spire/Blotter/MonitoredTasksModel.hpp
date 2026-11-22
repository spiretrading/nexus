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

      /** Observes a source task list with local pins and monitors. */
      MonitoredTasksModel(std::shared_ptr<TasksModel> source,
        std::shared_ptr<TaskMonitorsModel> monitors);

      QValidator::State set(int index, const Type& value) override;
      QValidator::State insert(const Type& value, int index) override;
      QValidator::State move(int source, int destination) override;
      QValidator::State remove(int index) override;
      using ArrayListModel::insert;
      using ArrayListModel::remove;

    private:
      std::shared_ptr<TasksModel> m_source;
      std::shared_ptr<TaskMonitorsModel> m_monitors;
      boost::signals2::scoped_connection m_connection;
      boost::signals2::scoped_connection m_source_connection;

      BlotterTaskEntry observe(BlotterTaskEntry entry) const;
      void on_monitor(const TaskMonitorsModel::Operation& operation);
      void on_source(const TasksModel::Operation& operation);
  };
}

#endif
