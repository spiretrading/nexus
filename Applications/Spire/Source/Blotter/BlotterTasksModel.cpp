#include "Spire/Blotter/BlotterTasksModel.hpp"
#include <unordered_set>
#include <Beam/Queues/FilteredQueueReader.hpp>
#include <Beam/Queues/QueueReaderPublisher.hpp>
#include <Beam/Queues/StateQueue.hpp>
#include "Nexus/OrderExecutionService/StandardQueries.hpp"
#include "Spire/Blotter/BlotterModelUtilities.hpp"
#include "Spire/Blotter/BlotterTaskMonitor.hpp"
#include "Spire/Canvas/OrderExecutionNodes/OrderWrapperTaskNode.hpp"
#include "Spire/Canvas/SystemNodes/CanvasObserver.hpp"
#include "Spire/UI/CustomQtVariants.hpp"
#include "Spire/UI/UserProfile.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Reactors;
using namespace Beam::Routines;
using namespace Beam::ServiceLocator;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Spire;

namespace {
  const auto EXPIRY_INTERVAL = 500;

  RoutineHandler QueryDailyOrderSubmissions(const UserProfile& userProfile,
      const DirectoryEntry& account, ScopedQueueWriter<const Order*> queue) {
    return Spawn([&userProfile, account, queue = std::move(queue)] () mutable {
      auto currentTime =
        userProfile.GetServiceClients().GetTimeClient().GetTime();
      auto lastSequence = Beam::Queries::Sequence::First();
      auto timeOfDay =
        userProfile.GetServiceClients().GetTimeClient().GetTime();
      for(auto& market : userProfile.GetMarketDatabase().GetEntries()) {
        auto snapshotQuery = MakeDailyOrderSubmissionQuery(market.m_code,
          account, timeOfDay, timeOfDay, userProfile.GetMarketDatabase(),
          userProfile.GetTimeZoneDatabase());
        auto snapshotQueue = std::make_shared<Queue<SequencedOrder>>();
        userProfile.GetServiceClients().GetOrderExecutionClient().
          QueryOrderSubmissions(snapshotQuery, snapshotQueue);
        try {
          while(true) {
            auto value = snapshotQueue->Pop();
            queue.Push(value.GetValue());
            lastSequence = std::max(lastSequence, value.GetSequence());
          }
        } catch(const std::exception&) {}
      }
      auto query = AccountQuery();
      query.SetIndex(account);
      query.SetSnapshotLimit(SnapshotLimit::Unlimited());
      if(lastSequence == Beam::Queries::Sequence::First()) {
        query.SetRange(currentTime, pos_infin);
      } else {
        query.SetRange(Increment(lastSequence), pos_infin);
      }
      query.SetInterruptionPolicy(InterruptionPolicy::RECOVER_DATA);
      userProfile.GetServiceClients().GetOrderExecutionClient().
        QueryOrderSubmissions(query, std::move(queue));
    });
  }

  struct UniqueFilter {
    std::unordered_set<const Order*> m_orders;

    bool operator ()(const Order* order) {
      return m_orders.insert(order).second;
    }
  };
}

BlotterTasksModel::ObserverEntry::ObserverEntry(std::string name,
  std::unique_ptr<CanvasObserver> observer)
  : m_name(std::move(name)),
    m_observer(std::move(observer)) {}

BlotterTasksModel::BlotterTasksModel(Ref<UserProfile> userProfile,
    const DirectoryEntry& executingAccount, bool isConsolidated,
    const BlotterTaskProperties& properties)
    : m_userProfile(userProfile.Get()),
      m_executingAccount(executingAccount),
      m_properties(properties),
      m_isRefreshing(false) {
  m_taskEventHandler.emplace();
  if(isConsolidated) {
    auto orderQueue = std::make_shared<Queue<const Order*>>();
    m_pendingRoutines.Add(QueryDailyOrderSubmissions(*m_userProfile,
      m_executingAccount, orderQueue));
    m_accountOrderPublisher = MakeSequencePublisherAdaptor(
      std::make_shared<QueueReaderPublisher<const Order*>>(orderQueue));
  } else {
    m_accountOrderPublisher =
      std::make_shared<SequencePublisher<const Order*>>();
  }
  m_accountOrderPublisher->Monitor(m_orderEventHandler.get_slot<const Order*>(
    std::bind_front(&BlotterTasksModel::OnOrderSubmitted, this)));
  SetupLinkedOrderExecutionMonitor();
  connect(&m_expiryTimer, &QTimer::timeout, this,
    &BlotterTasksModel::OnExpiryTimer);
  m_expiryTimer.start(EXPIRY_INTERVAL);
  for(auto& monitor : m_properties.GetMonitors()) {
    m_taskMonitors.push_back(monitor);
  }
}

const BlotterTaskProperties& BlotterTasksModel::GetProperties() const {
  return m_properties;
}

void BlotterTasksModel::SetProperties(const BlotterTaskProperties& properties) {
  m_properties = properties;
  m_taskMonitors.clear();
  for(auto& monitor : m_properties.GetMonitors()) {
    m_taskMonitors.push_back(monitor);
  }
  Refresh();
}

const OrderExecutionPublisher&
    BlotterTasksModel::GetOrderExecutionPublisher() const {
  return *m_linkedOrderExecutionPublisher;
}

const BlotterTasksModel::TaskEntry& BlotterTasksModel::Add(
    const CanvasNode& node) {
  return Add(
    std::make_shared<Task>(node, m_executingAccount, Ref(*m_userProfile)));
}

const BlotterTasksModel::TaskEntry& BlotterTasksModel::Add(
    std::shared_ptr<Task> task) {
  auto taskIterator = m_taskIds.find(task->GetId());
  if(taskIterator != m_taskIds.end()) {
    return *(taskIterator->second);
  }
  auto index = static_cast<int>(m_entries.size());
  beginInsertRows(QModelIndex(), index, index);
  auto entry = std::make_unique<TaskEntry>();
  entry->m_index = index;
  entry->m_sticky = false;
  entry->m_task = std::move(task);
  entry->m_task->GetPublisher().Monitor(
    m_taskEventHandler->get_slot<Task::StateEntry>(
      [=, entry = entry.get()] (const auto& state) {
        OnTaskState(*entry, state);
      }));
  m_taskIds.insert(std::pair(entry->m_task->GetId(), entry.get()));
  for(auto& taskMonitor : m_taskMonitors) {
    auto observer =
      std::make_unique<CanvasObserver>(entry->m_task, taskMonitor.GetMonitor());
    auto observerEntry = std::make_unique<ObserverEntry>(
      taskMonitor.GetName(), std::move(observer));
    observerEntry->m_connection =
      observerEntry->m_observer->ConnectUpdateSignal(
        [=, entry = entry.get(), name = taskMonitor.GetName()] (
            const any& value) {
          OnMonitorUpdate(*entry, name, value);
        });
    entry->m_monitors.push_back(std::move(observerEntry));
  }
  auto& entryReference = *entry;
  m_entries.push_back(std::move(entry));
  endInsertRows();
  m_taskAddedSignal(entryReference);
  for(auto& link : m_outgoingLinks) {
    link->Add(entryReference.m_task);
  }
  entryReference.m_task->GetContext().GetOrderPublisher().Monitor(
    m_orders->GetWriter());
  entryReference.m_task->GetContext().GetOrderPublisher().Monitor(
    m_orderEventHandler.get_slot<const Order*>(
      std::bind_front(&BlotterTasksModel::OnTaskOrderSubmitted, this)));
  return entryReference;
}

const std::vector<BlotterTasksModel*>&
    BlotterTasksModel::GetIncomingLinks() const {
  return m_incomingLinks;
}

void BlotterTasksModel::Refresh() {
  if(m_isRefreshing) {
    return;
  }
  beginResetModel();
  m_isRefreshing = true;
  if(!m_entries.empty()) {
    beginRemoveRows(QModelIndex(), 0, m_entries.size() - 1);
    auto entries = std::vector<std::unique_ptr<TaskEntry>>();
    entries.swap(m_entries);
    m_pendingExpiryEntries.clear();
    m_expiredEntries.clear();
    m_taskEventHandler = std::nullopt;
    m_taskEventHandler.emplace();
    m_taskIds.clear();
    m_submittedOrders.clear();
    m_taskOrders.clear();
    endRemoveRows();
    for(auto& entry : entries) {
      m_taskRemovedSignal(*entry);
    }
  }
  for(auto& link : m_outgoingLinks) {
    link->Refresh();
  }
  WithModels(*this, [&] (const BlotterTasksModel& model) {
    model.GetOrderExecutionPublisher().Monitor(m_orders->GetWriter());
    auto size = model.rowCount();
    for(auto i = 0; i != size; ++i) {
      auto& entry = model.GetEntry(i);
      Add(entry.m_task);
    }
  });
  m_isRefreshing = false;
  endResetModel();
}

void BlotterTasksModel::Link(Ref<BlotterTasksModel> model) {
  model->GetOrderExecutionPublisher().Monitor(m_orders->GetWriter());
  m_incomingLinks.push_back(model.Get());
  model->m_outgoingLinks.push_back(this);
  for(const auto& link : m_outgoingLinks) {
    link->Refresh();
  }
  WithModels(*model.Get(), [&] (const BlotterTasksModel& model) {
    auto size = model.rowCount();
    for(auto i = 0; i != size; ++i) {
      auto& entry = model.GetEntry(i);
      Add(entry.m_task);
    }
  });
}

void BlotterTasksModel::Unlink(BlotterTasksModel& model) {
  m_incomingLinks.erase(
    find(m_incomingLinks.begin(), m_incomingLinks.end(), &model));
  model.m_outgoingLinks.erase(find(model.m_outgoingLinks.begin(),
    model.m_outgoingLinks.end(), this));
  SetupLinkedOrderExecutionMonitor();
  Refresh();
}

const BlotterTasksModel::TaskEntry&
    BlotterTasksModel::GetEntry(int index) const {
  return *m_entries[index];
}

connection BlotterTasksModel::ConnectTaskAddedSignal(
    const TaskAddedSignal::slot_function_type& slot) const {
  return m_taskAddedSignal.connect(slot);
}

connection BlotterTasksModel::ConnectTaskRemovedSignal(
    const TaskRemovedSignal::slot_function_type& slot) const {
  return m_taskRemovedSignal.connect(slot);
}

int BlotterTasksModel::rowCount(const QModelIndex& parent) const {
  return static_cast<int>(m_entries.size());
}

int BlotterTasksModel::columnCount(const QModelIndex& parent) const {
  return STANDARD_COLUMN_COUNT + m_taskMonitors.size();
}

QVariant BlotterTasksModel::data(const QModelIndex& index, int role) const {
  if(!index.isValid()) {
    return QVariant();
  }
  auto& entry = *m_entries[index.row()];
  if(role == Qt::TextAlignmentRole) {
    return static_cast<int>(Qt::AlignHCenter | Qt::AlignVCenter);
  } else if(role == Qt::DisplayRole) {
    if(index.column() == STICKY_COLUMN) {
      return entry.m_sticky;
    } else if(index.column() == NAME_COLUMN) {
      return QString::fromStdString(entry.m_task->GetNode().GetText());
    } else if(index.column() == ID_COLUMN) {
      return entry.m_task->GetId();
    } else if(index.column() == STATE_COLUMN) {
      return QVariant::fromValue(entry.m_state);
    } else if(index.column() - STANDARD_COLUMN_COUNT >= 0 &&
        index.column() - STANDARD_COLUMN_COUNT < entry.m_monitors.size()) {
      auto value = entry.m_monitors[
        index.column() - STANDARD_COLUMN_COUNT]->m_observer->GetValue();
      return QVariant::fromValue(value);
    } else {
      return QVariant();
    }
  }
  return QVariant();
}

QVariant BlotterTasksModel::headerData(
    int section, Qt::Orientation orientation, int role) const {
  if(role == Qt::TextAlignmentRole) {
    return static_cast<int>(Qt::AlignHCenter | Qt::AlignVCenter);
  } else if(role == Qt::DisplayRole) {
    if(section == STICKY_COLUMN) {
      return QString("");
    } else if(section == NAME_COLUMN) {
      return QString("Name");
    } else if(section == ID_COLUMN) {
      return QString("ID");
    } else if(section == STATE_COLUMN) {
      return QString("State");
    } else if(section - STANDARD_COLUMN_COUNT >= 0 &&
        section - STANDARD_COLUMN_COUNT < m_taskMonitors.size()) {
      return QString::fromStdString(
        m_taskMonitors[section - STANDARD_COLUMN_COUNT].GetName());
    }
  }
  return QVariant();
}

bool BlotterTasksModel::setData(
    const QModelIndex& index, const QVariant& value, int role) {
  if(!index.isValid() || index.column() != STICKY_COLUMN ||
      role != Qt::EditRole || !value.canConvert<bool>()) {
    return false;
  }
  auto& entry = m_entries[index.row()];
  if(entry->m_sticky == value.value<bool>()) {
    return false;
  }
  entry->m_sticky = value.value<bool>();
  if(!entry->m_sticky && IsTerminal(entry->m_state)) {
    m_pendingExpiryEntries.insert(entry.get());
  }
  Q_EMIT dataChanged(index, index);
  return true;
}

void BlotterTasksModel::SetupLinkedOrderExecutionMonitor() {
  m_orders = std::make_shared<MultiQueueWriter<const Order*>>();
  m_linkedOrderExecutionPublisher = MakeSequencePublisherAdaptor(
    std::make_shared<QueueReaderPublisher<const Order*>>(
      MakeFilteredQueueReader(m_orders, UniqueFilter())));
  m_accountOrderPublisher->Monitor(m_orders->GetWriter());
}

void BlotterTasksModel::OnMonitorUpdate(TaskEntry& entry,
    const std::string& property, const any& value) {
  Q_EMIT dataChanged(index(entry.m_index, 0),
    index(entry.m_index, columnCount(QModelIndex()) - 1));
}

void BlotterTasksModel::OnTaskState(TaskEntry& entry,
    const Task::StateEntry& update) {
  entry.m_state = update.m_state;
  Q_EMIT dataChanged(index(entry.m_index, 0),
    index(entry.m_index, columnCount(QModelIndex()) - 1));
  if(!entry.m_sticky && IsTerminal(entry.m_state)) {
    m_pendingExpiryEntries.insert(&entry);
  }
}

void BlotterTasksModel::OnOrderSubmitted(const Order* order) {
  if(m_taskOrders.count(order) != 0) {
    m_taskOrders.erase(order);
    return;
  } else if(m_submittedOrders.insert(order).second) {
    m_orderEventHandler.push([=] {
      OnOrderSubmitted(order);
    });
    return;
  }
  m_submittedOrders.erase(order);
  auto reportQueue = std::make_shared<StateQueue<ExecutionReport>>();
  order->GetPublisher().Monitor(reportQueue);
  if(auto report = reportQueue->Peek(); IsTerminal(report.m_status)) {
    m_orders->Push(order);
    return;
  }
  auto& entry = Add(OrderWrapperTaskNode(*order, *m_userProfile));
  entry.m_task->Execute();
}

void BlotterTasksModel::OnTaskOrderSubmitted(const Order* order) {
  m_taskOrders.insert(order);
}

void BlotterTasksModel::OnExpiryTimer() {
  for(auto& entry : m_expiredEntries) {
    auto index = entry->m_index;
    if(index == -1) {
      continue;
    }
    entry->m_index = -1;
    beginRemoveRows(QModelIndex(), index, index);
    for(auto j = std::size_t(index + 1); j < m_entries.size(); ++j) {
      assert(m_entries[j]->m_index > 0);
      --(m_entries[j]->m_index);
    }
    m_entries.erase(m_entries.begin() + index);
    endRemoveRows();
    m_taskRemovedSignal(*entry);
  }
  m_expiredEntries.clear();
  m_expiredEntries.insert(m_expiredEntries.begin(),
    m_pendingExpiryEntries.begin(), m_pendingExpiryEntries.end());
  m_pendingExpiryEntries.clear();
}
