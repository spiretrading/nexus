#ifndef SPIRE_BLOTTERTASKSMODEL_HPP
#define SPIRE_BLOTTERTASKSMODEL_HPP
#include <set>
#include <unordered_set>
#include <Beam/Pointers/DelayPtr.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Queues/TaskQueue.hpp>
#include <Beam/Threading/Sync.hpp>
#include <boost/noncopyable.hpp>
#include <boost/signals2/signal.hpp>
#include <QAbstractItemModel>
#include <QTimer>
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/Blotter/BlotterTaskProperties.hpp"
#include "Spire/Canvas/Operations/CanvasNodeTranslationContext.hpp"
#include "Spire/Canvas/TaskNodes/Task.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /*! \class BlotterTasksModel
      \brief Model used for a BlotterWindow's Tasks.
   */
  class BlotterTasksModel : public QAbstractTableModel {
    public:

      /*! \enum Columns
          \brief Enumerates the standard columns displayed in the Tasks table.
       */
      enum Columns {

        //! Whether the Task is sticky.
        STICKY_COLUMN,

        //! The Task's name.
        NAME_COLUMN,

        //! The Task's id.
        ID_COLUMN,

        //! The State of the Task.
        STATE_COLUMN
      };

      //! The number of standard columns.
      static const unsigned int STANDARD_COLUMN_COUNT = 4;

      /*! \struct TaskContext
          \brief Stores a Task and the context needed to execute.
       */
      struct TaskContext : private boost::noncopyable {

        //! The ReactorMonitor used by the Tasks.
        Beam::Reactors::ReactorMonitor m_reactorMonitor;

        //! The Task's context.
        CanvasNodeTranslationContext m_context;

        //! The node used to build the Task.
        std::unique_ptr<CanvasNode> m_node;

        //! The Task's OrderExecutionPublisher.
        std::shared_ptr<Nexus::OrderExecutionService::OrderExecutionPublisher>
          m_orderExecutionPublisher;

        //! The Task that was built.
        std::shared_ptr<Task> m_task;

        //! Constructs a TaskContext.
        /*!
          \param userProfile The user's profile.
          \param node The node used to build the Task.
          \param executingAccount The account executing Orders.
        */
        TaskContext(Beam::Ref<UserProfile> userProfile,
          const CanvasNode& node,
          const Beam::ServiceLocator::DirectoryEntry& executingAccount);
      };

      /*! \struct ObserverEntry
          \brief Stores a CanvasObserver for an executing Task.
       */
      struct ObserverEntry : private boost::noncopyable {

        //! The name of the monitor.
        std::string m_name;

        //! The observer monitoring the Task.
        std::unique_ptr<CanvasObserver> m_observer;

        //! The connection to the CanvasObserver.
        boost::signals2::scoped_connection m_connection;

        //! Constructs an ObserverEntry.
        /*!
          \param name The name of the monitor.
          \param observer The CanvasObserver monitoring the CanvasNode.
        */
        ObserverEntry(std::string name,
          std::unique_ptr<CanvasObserver> observer);
      };

      /*! \struct TaskEntry
          \brief Stores information about a single Task entry.
       */
      struct TaskEntry : private boost::noncopyable {

        //! The index of this entry.
        int m_index;

        //! Whether the Task is sticky.
        bool m_sticky;

        //! The Task and its context.
        std::shared_ptr<TaskContext> m_context;

        //! The current state of the Task.
        Task::State m_state;

        //! The properties monitoring the Task.
        std::vector<std::unique_ptr<ObserverEntry>> m_monitors;
      };

      //! Signals a Task was added to this model.
      /*!
        \param entry The TaskEntry that was added.
      */
      typedef boost::signals2::signal<void (const TaskEntry& entry)>
        TaskAddedSignal;

      //! Signals a Task was removed from this model.
      /*!
        \param entry The TaskEntry that was removed.
      */
      typedef boost::signals2::signal<void (const TaskEntry& entry)>
        TaskRemovedSignal;

      //! Constructs a BlotterTasksModel.
      /*!
        \param userProfile The user's profile.
        \param executingAccount The account used to execute new Orders.
        \param isConsolidated Whether this model consolidates all of the
               <i>executingAccount<i>'s Orders.
        \param properties The properties used to display Tasks.
      */
      BlotterTasksModel(Beam::Ref<UserProfile> userProfile,
        const Beam::ServiceLocator::DirectoryEntry& executingAccount,
        bool isConsolidated, const BlotterTaskProperties& properties);

      virtual ~BlotterTasksModel();

      //! Returns the BlotterTaskProperties used by this model.
      const BlotterTaskProperties& GetProperties() const;

      //! Sets the BlotterTaskProperties to use.
      /*!
        \param properties The BlotterTaskProperties to use.
      */
      void SetProperties(const BlotterTaskProperties& properties);

      //! Returns the OrderExecutionPublisher.
      Nexus::OrderExecutionService::OrderExecutionPublisher&
        GetOrderExecutionPublisher() const;

      //! Adds a CanvasNode to the model.
      /*!
        \param node The node to add.
        \return The TaskEntry for the newly entered node.
      */
      const TaskEntry& Add(const CanvasNode& node);

      //! Adopts an existing TaskContext, assuming ownership of it.
      /*!
        \param context The context to adopt.
        \return The TaskEntry for the adopted <i>context</i>.
      */
      const TaskEntry& Adopt(std::unique_ptr<TaskContext> context);

      //! Monitors a Task.
      /*!
        \param context The TaskContext containing the Task to monitor.
      */
      void Monitor(const std::shared_ptr<TaskContext>& context);

      //! Returns all TaskContexts strictly belonging to this model.
      const std::vector<std::shared_ptr<TaskContext>>& GetContexts() const;

      //! Returns all models linking into this one.
      const std::vector<BlotterTasksModel*>& GetIncomingLinks() const;

      //! Rebuilds the contents of this model.
      void Refresh();

      //! Links another BlotterTasksModel to this.
      /*!
        \param model The BlotterTasksModel to link to this one.
      */
      void Link(Beam::Ref<BlotterTasksModel> model);

      //! Unlinks an BlotterTasksModel from this one.
      /*!
        \param model The BlotterTasksModel to unlink from this.
      */
      void Unlink(BlotterTasksModel& model);

      //! Returns the TaskEntry at a specified index.
      /*!
        \param index The index.
        \return The TaskEntry at the specified <i>index</i>.
      */
      const TaskEntry& GetEntry(int index) const;

      //! Connects a slot to the TaskAddedSignal.
      /*!
        \param slot The slot to connect to the TaskAddedSignal.
        \return A connection to the specified signal.
      */
      boost::signals2::connection ConnectTaskAddedSignal(
        const TaskAddedSignal::slot_function_type& slot) const;

      //! Connects a slot to the TaskRemovedSignal.
      /*!
        \param slot The slot to connect to the TaskRemovedSignal.
        \return A connection to the specified signal.
      */
      boost::signals2::connection ConnectTaskRemovedSignal(
        const TaskRemovedSignal::slot_function_type& slot) const;

      virtual int rowCount(const QModelIndex& parent) const;

      virtual int columnCount(const QModelIndex& parent) const;

      virtual QVariant data(const QModelIndex& index, int role) const;

      virtual QVariant headerData(int section, Qt::Orientation orientation,
        int role) const;

      virtual bool setData(const QModelIndex& index, const QVariant& value,
        int role = Qt::EditRole);

    private:
      UserProfile* m_userProfile;
      Beam::ServiceLocator::DirectoryEntry m_executingAccount;
      bool m_isConsolidated;
      BlotterTaskProperties m_properties;
      QTimer m_updateTimer;
      QTimer m_expiryTimer;
      bool m_isRefreshing;
      SpireAggregateOrderExecutionPublisher m_properOrderExecutionPublisher;
      Beam::DelayPtr<SpireAggregateOrderExecutionPublisher>
        m_linkedOrderExecutionPublisher;
      std::vector<std::shared_ptr<TaskContext>> m_contexts;
      std::unordered_set<long long> m_taskIds;
      std::vector<BlotterTaskMonitor> m_taskMonitors;
      std::vector<std::shared_ptr<TaskEntry>> m_entries;
      std::set<std::shared_ptr<TaskEntry>> m_pendingExpiryEntries;
      std::vector<std::shared_ptr<TaskEntry>> m_expiredEntries;
      std::unordered_set<Nexus::OrderExecutionService::OrderId>
        m_submittedOrders;
      std::vector<BlotterTasksModel*> m_incomingLinks;
      std::vector<BlotterTasksModel*> m_outgoingLinks;
      std::shared_ptr<Nexus::OrderExecutionService::OrderExecutionPublisher>
        m_accountOrderPublisher;
      mutable TaskAddedSignal m_taskAddedSignal;
      mutable TaskRemovedSignal m_taskRemovedSignal;
      Beam::TaskQueue m_orderSlotHandler;
      Beam::DelayPtr<Beam::TaskQueue> m_taskSlotHandler;

      void SetupLinkedOrderExecutionMonitor();
      void OnMonitorUpdate(std::weak_ptr<TaskEntry> weakEntry,
        const std::string& property, const boost::any& value);
      void OnTaskState(std::weak_ptr<TaskEntry> weakEntry,
        const Task::StateEntry& update);
      void OnOrderSubmitted(const Nexus::OrderExecutionService::Order* order);
      void OnOrderExecuted(const Nexus::OrderExecutionService::Order* order);
      void OnUpdateTimer();
      void OnExpiryTimer();
  };
}

#endif
