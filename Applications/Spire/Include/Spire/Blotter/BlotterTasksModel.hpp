#ifndef SPIRE_BLOTTER_TASKS_MODEL_HPP
#define SPIRE_BLOTTER_TASKS_MODEL_HPP
#include <optional>
#include <set>
#include <unordered_map>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Queues/MultiQueueWriter.hpp>
#include <Beam/Queues/TaskQueue.hpp>
#include <Beam/Routines/RoutineHandlerGroup.hpp>
#include <boost/any.hpp>
#include <boost/noncopyable.hpp>
#include <boost/signals2/signal.hpp>
#include <QAbstractItemModel>
#include <QTimer>
#include "Spire/Async/EventHandler.hpp"
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/Blotter/BlotterTaskProperties.hpp"
#include "Spire/Canvas/SystemNodes/CanvasObserver.hpp"
#include "Spire/Canvas/Tasks/Task.hpp"
#include "Spire/UI/UI.hpp"

namespace Spire {

  /** Model used for a BlotterWindow's Tasks. */
  class BlotterTasksModel : public QAbstractTableModel {
    public:

      /** Enumerates the standard columns displayed in the Tasks table. */
      enum Columns {

        /** Whether the Task is sticky. */
        STICKY_COLUMN,

        /** The Task's name. */
        NAME_COLUMN,

        /** The Task's id. */
        ID_COLUMN,

        /** The State of the Task. */
        STATE_COLUMN
      };

      /** The number of standard columns. */
      static constexpr unsigned int STANDARD_COLUMN_COUNT = 4;

      /** Stores a CanvasObserver for an executing Task. */
      struct ObserverEntry : private boost::noncopyable {

        /** The name of the monitor. */
        std::string m_name;

        /** The observer monitoring the Task. */
        std::unique_ptr<CanvasObserver> m_observer;

        /** The connection to the CanvasObserver. */
        boost::signals2::scoped_connection m_connection;

        /**
         * Constructs an ObserverEntry.
         * @param name The name of the monitor.
         * @param observer The CanvasObserver monitoring the CanvasNode.
         */
        ObserverEntry(std::string name,
          std::unique_ptr<CanvasObserver> observer);
      };

      /** Stores information about a single Task entry. */
      struct TaskEntry {

        /** The index of this entry. */
        int m_index;

        /** Whether the Task is sticky. */
        bool m_sticky;

        /** The Task represented. */
        std::shared_ptr<Task> m_task;

        /** The current state of the Task. */
        Task::State m_state;

        /** The properties monitoring the Task. */
        std::vector<std::unique_ptr<ObserverEntry>> m_monitors;
      };

      /**
       * Signals a Task was added to this model.
       * @param entry The TaskEntry that was added.
       */
      using TaskAddedSignal = boost::signals2::signal<
        void (const TaskEntry& entry)>;

      /**
       * Signals a Task was removed from this model.
       * @param entry The TaskEntry that was removed.
       */
      using TaskRemovedSignal = boost::signals2::signal<
        void (const TaskEntry& entry)>;

      /**
       * Constructs a BlotterTasksModel.
       * @param userProfile The user's profile.
       * @param executingAccount The account used to execute new Orders.
       * @param isConsolidated Whether this model consolidates all of the
       *        <i>executingAccount<i>'s Orders.
       * @param properties The properties used to display Tasks.
       */
      BlotterTasksModel(Beam::Ref<UserProfile> userProfile,
        const Beam::DirectoryEntry& executingAccount,
        bool isConsolidated, const BlotterTaskProperties& properties);

      ~BlotterTasksModel() = default;

      /** Returns the BlotterTaskProperties used by this model. */
      const BlotterTaskProperties& GetProperties() const;

      /**
       * Sets the BlotterTaskProperties to use.
       * @param properties The BlotterTaskProperties to use.
       */
      void SetProperties(const BlotterTaskProperties& properties);

      /** Returns the OrderExecutionPublisher. */
      const Beam::Publisher<std::shared_ptr<Nexus::Order>>&
        GetOrderExecutionPublisher() const;

      /**
       * Adds a new task defined by a canvas node to the model.
       * @param node The node used to add the new task.
       * @return The TaskEntry for the newly created task.
       */
      const TaskEntry& Add(const CanvasNode& node);

      /**
       * Adds an existing Task to the model.
       * @param task The task to add.
       * @return The TaskEntry representing the added task.
       */
      const TaskEntry& Add(std::shared_ptr<Task> task);

      /** Returns all models linking into this one. */
      const std::vector<BlotterTasksModel*>& GetIncomingLinks() const;

      /** Rebuilds the contents of this model. */
      void Refresh();

      /**
       * Links another BlotterTasksModel to this.
       * @param model The BlotterTasksModel to link to this one.
       */
      void Link(Beam::Ref<BlotterTasksModel> model);

      /**
       * Unlinks an BlotterTasksModel from this one.
       * @param model The BlotterTasksModel to unlink from this.
       */
      void Unlink(BlotterTasksModel& model);

      /**
       * Returns the TaskEntry at a specified index.
       * @param index The index.
       * @return The TaskEntry at the specified <i>index</i>.
       */
      const TaskEntry& GetEntry(int index) const;

      /**
       * Connects a slot to the TaskAddedSignal.
       * @param slot The slot to connect to the TaskAddedSignal.
       * @return A connection to the specified signal.
       */
      boost::signals2::connection ConnectTaskAddedSignal(
        const TaskAddedSignal::slot_function_type& slot) const;

      /**
       * Connects a slot to the TaskRemovedSignal.
       * @param slot The slot to connect to the TaskRemovedSignal.
       * @return A connection to the specified signal.
       */
      boost::signals2::connection ConnectTaskRemovedSignal(
        const TaskRemovedSignal::slot_function_type& slot) const;

      int rowCount(const QModelIndex& parent = QModelIndex()) const override;

      int columnCount(const QModelIndex& parent = QModelIndex()) const override;

      QVariant data(const QModelIndex& index, int role) const override;

      QVariant headerData(int section, Qt::Orientation orientation,
        int role) const override;

      bool setData(const QModelIndex& index, const QVariant& value,
        int role = Qt::EditRole) override;

    private:
      UserProfile* m_userProfile;
      Beam::DirectoryEntry m_executingAccount;
      BlotterTaskProperties m_properties;
      QTimer m_expiryTimer;
      bool m_isRefreshing;
      std::shared_ptr<Beam::MultiQueueWriter<std::shared_ptr<Nexus::Order>>>
        m_orders;
      std::shared_ptr<Beam::Publisher<std::shared_ptr<Nexus::Order>>>
        m_linkedOrderExecutionPublisher;
      std::vector<std::unique_ptr<TaskEntry>> m_entries;
      std::unordered_map<int, TaskEntry*> m_taskIds;
      std::vector<BlotterTaskMonitor> m_taskMonitors;
      std::set<TaskEntry*> m_pendingExpiryEntries;
      std::vector<TaskEntry*> m_expiredEntries;
      std::vector<BlotterTasksModel*> m_incomingLinks;
      std::vector<BlotterTasksModel*> m_outgoingLinks;
      std::shared_ptr<Beam::Publisher<std::shared_ptr<Nexus::Order>>>
        m_accountOrderPublisher;
      std::set<std::shared_ptr<Nexus::Order>> m_submittedOrders;
      std::set<std::shared_ptr<Nexus::Order>> m_taskOrders;
      mutable TaskAddedSignal m_taskAddedSignal;
      mutable TaskRemovedSignal m_taskRemovedSignal;
      EventHandler m_orderEventHandler;
      std::optional<EventHandler> m_taskEventHandler;
      Beam::RoutineHandlerGroup m_pendingRoutines;

      void SetupLinkedOrderExecutionMonitor();
      void OnMonitorUpdate(TaskEntry& entry, const std::string& property,
        const boost::any& value);
      void OnTaskState(TaskEntry& entry, const Task::StateEntry& update);
      void OnOrderSubmitted(const std::shared_ptr<Nexus::Order>& order);
      void OnTaskOrderSubmitted(const std::shared_ptr<Nexus::Order>& order);
      void OnExpiryTimer();
  };
}

#endif
